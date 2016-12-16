#include <glad/glad.h>
#include <graphics/graphics.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utils/utils.h"
#include "globals/globals.h"

void gen_buffers(
                 GLuint num_buffers,
                 VBO * vbo,
                 Point_Data * data,
                 GLuint draw_type
                )
{
    /* Generate num_buffers to GLuint * buffer pointer. */

    // Generate number of buffers.
    glGenBuffers(num_buffers, &vbo->vbo);
    // Bind buffer.
    glBindBuffer(GL_ARRAY_BUFFER, vbo->vbo);
    // Load data to buffer.
    size_t data_size = sizeof(float) * data->elements;
    glBufferData(GL_ARRAY_BUFFER, data_size, data->data, draw_type);
    // Unbind buffer.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // Store point data to vbo.
    vbo->point_data = data;
    vbo->draw_type = draw_type;
}

void gen_vertex_arrays(
                       GLuint num_buffers,
                       VAO * vao,
                       VBO * vbo_binds[],
                       size_t num_vbos,
                       Attrib_Pointer_Info * enable_arrayp,
                       size_t num_arrayp
                      )
{
    /* Generate num_buffer vertex array buffers. Bind all the vbos in the
     * vbo_bind list. Enable all Vertex Attrib Array pointers in
     * enabled_arrayp.
     */

    // Generate vertex array buffers.
    glGenVertexArrays(num_buffers, &vao->vao);

    // Bind the vertex array buffer.
    glBindVertexArray(vao->vao);

    // Bind all vbos in vbo_binds.
    for (size_t i=0; i<num_vbos; i++) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_binds[i]->vbo);
    }

    // Assume we only have one vbo for the time being.
    Point_Data * vbo_data = vbo_binds[0]->point_data;
    int items_per_row = vbo_data->elements / vbo_data->rows;

    // The stride is always the same?
    size_t stride = items_per_row*sizeof(GL_FLOAT);

    // Keep count of current offset.
    size_t offset = 0;

    // Enable all vertex attribute array pointers in enabled_arrayp.
    for (size_t i=0; i<num_arrayp; i++) {
        Attrib_Pointer_Info * arrayp = &enable_arrayp[i];
        glEnableVertexAttribArray(arrayp->index);
        glVertexAttribPointer(arrayp->index,
                              arrayp->size,
                              GL_FLOAT,
                              GL_FALSE,
                              stride,
                              (GLvoid *)(sizeof(GLfloat)*offset));
        // Increment offset for next in list.
        offset += arrayp->size;
    }

    // Store array inside of VAO.
    vao->attrib_list = enable_arrayp;
    vao->list_size = num_arrayp;

    // Calculate bounding box.

    GLfloat max_x = 0;
    GLfloat max_y = 0;
    GLfloat max_z = 0;
    GLfloat min_x = 0;
    GLfloat min_y = 0;
    GLfloat min_z = 0;

    GLfloat * point_data = vbo_binds[0]->point_data->data;
    for(size_t i=0; i<vbo_binds[0]->point_data->elements; i += items_per_row) {

        GLfloat x = point_data[i];
        GLfloat y = point_data[i+1];
        GLfloat z = point_data[i+2];

        if (x > max_x) {
            max_x = x;
        }
        if (x < min_x) {
            min_x = x;
        }
        if (y > max_y) {
            max_y = y;
        }
        if (y < min_y) {
            min_y = y;
        }
        if (z > max_z) {
            max_z = z;
        }
        if (z < min_z) {
            min_z = z;
        }
    }

    // Save bounding box to VAO struct as 8 * 3 = 24 floats. The first 4 * 3
    // points is the "front", beginning with the top left corner and going
    // around clockwise. The other 4 * 3 are for the "back" and are arrange in
    // the same order.
    //
    // (min_x, max_y, max_z) ->  *------*
    //                          /|     /|
    //                         *------* | (Ignore perspective)
    //                         | .____|.|
    //                         |/     |/
    //                         *------* <- (max_x, min_y, min_z)
    //
    // ( remember that the z-axis is inverted in OpenGL, negative z are closer
    //   to the "front". )

    v3 * bounds = vao->bounds;
    float order[] = {
        // Front plane.
        min_x, max_y, min_z, // top front left.
        max_x, max_y, min_z, // top front right.
        max_x, min_y, min_z, // bottom front right.
        min_x, min_y, min_z, // bottom front left.
        // Back plane.
        min_x, max_y, max_z, // top back left.
        max_x, max_y, max_z, // top back right.
        max_x, min_y, max_z, // bottom back right.
        min_x, min_y, max_z, // bottom back left.
    };

    // Use pointer arithmetic to write to the bounds vector array.
    for(size_t i=0; i<SIZE(order); i+=3) { // One row at the time.
        float * current_x = &order[i];
        float * current_vector = bounds[i/3];
        *current_vector = *current_x;
        *(current_vector+1) = *(current_x+1);
        *(current_vector+2) = *(current_x+2);
    }

    // Unbind the vertex array buffer.
    glBindVertexArray(0);
}

void create_shader(GLuint * shader, const char * source_filename)
{
    /* Create and return shader based on contents of file. */

    const char * error_base = "[!] create_shader: ";

    /* Determine the type based on the file extension. */
    GLuint type = 0;
    if (strstr(source_filename, VERTEX_ENDING) != NULL) {
        type = GL_VERTEX_SHADER;
    } else if (strstr(source_filename, FRAGMENT_ENDING) != NULL) {
        type = GL_FRAGMENT_SHADER;
    } else {
        fprintf(stderr, "%s Can't determine shader type of file: %s.\n",
                error_base,
                source_filename);
        exit(1);
    }

    *shader = glCreateShader(type);

    // Open and read size of file.
    long filesize = 0;
    FILE * filehandle = open_file(source_filename, &filesize);
    if (!filehandle) {
        fprintf(stderr, "%s Could not find shader source file: %s.\n",
                error_base,
                source_filename);
        exit(1);
    }

    char temp_buffer[filesize];
    char * string_p = temp_buffer;

    // Read data to temp_buffer.
    size_t read = read_file(temp_buffer, filesize, filehandle);
    if (!read) {
        fprintf(stderr, "%s No objects read from %s.\n",
                error_base,
                source_filename);
        exit(1);
    }

    glShaderSource(*shader, 1, &string_p, NULL);
    glCompileShader(*shader);
    checkShaderStatus(*shader);
}


void checkShaderStatus(GLuint shader) {
    GLint status = 0;

    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (status == GL_TRUE) {
        return;
    }

    GLint logSize = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);

    char message_buffer[logSize+1];

    glGetShaderInfoLog(shader, logSize, NULL, message_buffer);
    printf("Error in shader compilation: %s\n", message_buffer);
}

GLuint shader_program;

void link_program(GLuint * program, GLuint * shaders, size_t size) {

    // Create program.
    *program = glCreateProgram();

    // Attach all shaders in the list.
    for (size_t i=0; i<size; i++) {
        glAttachShader(*program, shaders[i]);
    }

    // Link program.
    glLinkProgram(*program);
}


void load_to_texture(GLuint * texture, const char * filename) {

    size_t width = 0;
    size_t height = 0;
    unsigned char * jpeg_data;

    load_image_data(&jpeg_data, filename, &height, &width);

    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, jpeg_data);

    free(jpeg_data);

    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}

/* #### Uniform related functions. */

void * uniform_data_transform(struct component * component)
    /* Get and return pointer to component transformation matrix. */
{
    return (void * )&component->transformation[0][0];
}

void * uniform_data_time(struct component * component)
    /* Get and return pointer to component transformation matrix. */
{
    UNUSED(component);
    return (void * )&global_variables[glfw_time];
}

void write_data_to_uniform(
                           struct uniform_data * uniform,
                           GLuint location,
                           void * data
                          )
    /* Use different functions depending on what type of uniform it is. */
{
    switch(uniform->type) {
        case UniformMatrix4fv:
            glUniformMatrix4fv(location,
                               1,
                               GL_TRUE,
                               data);
            break;
        case Uniform1f:
            glUniform1f(location, *(GLfloat * )data);
            break;
        default:
            fprintf(stderr, "Unrecognized uniform type, aborting.\n");
            exit(1);
    }
}

VAO * create_vao(Point_Data * point_data, GLuint draw_option, GLuint geometry)
    /* Take Point_Data pointer GLuint options and return a pointer to allocated
     * and populated VAO object. Automatically creates vao that conforms to the
     * vertex | color | texture-coord data format. */
{
    // Define VAO and VBO for vao and vbo.
    VAO * vao = malloc(sizeof(VAO));
    VBO * vbo = malloc(sizeof(VBO));

    // Generate buffers.
    gen_buffers(1, vbo, point_data, draw_option);

    // Set render type for vbo.
    vbo->render_geometry = geometry;

    // Set up attribute pointer information.
    Attrib_Pointer_Info attribs[] = {
        (Attrib_Pointer_Info){ // Vertex data.
            .index = 0,
            .size = 3,
        },
        (Attrib_Pointer_Info){ // Color data.
            .index = 1,
            .size = 3,
        },
        (Attrib_Pointer_Info){ // Texture coord data.
            .index = 2,
            .size = 2,
        },
    };

    // Set up the vbos that should be bound to the vao.
    VBO * vbo_binds[] = {
        vbo,
    };

    // Generate vertex array buffer.
    gen_vertex_arrays(
                      1,
                      vao,
                      vbo_binds,
                      SIZE(vbo_binds),
                      attribs,
                      SIZE(attribs)
                     );

    // Set vbo as vao.vbo.
    vao->vbo = *vbo;
    // Set start.
    vao->start = 0;
    // Set count.
    vao->count = vao->vbo.point_data->rows;

    return vao;
}

/* #### Component related functionality. */

void draw_component(
                    struct component * component,
                    GLuint program,
                    GLuint texture,
                    struct uniform_data * uniforms,
                    size_t num_uniforms
                   )
    /* Function that binds component vao and draws stored geometry with
     * supplied texture and shader program. */
{
    // Must have the program active when writing.
    glUseProgram(program);
    // Write to uniform location.

    // Bind texture.
    glBindTexture(GL_TEXTURE_2D, texture);

    struct uniform_data * current_uniform = NULL;
    // Iterate over and bind all uniforms.
    for (size_t i=0; i<num_uniforms; i++) {
        current_uniform = &uniforms[i];
        // Currently get location for shader_program every, unnecessary.
        GLuint uniform_location = glGetUniformLocation(program, current_uniform->name);
        // Write component transform to current shader program.
        write_data_to_uniform(current_uniform,
                              uniform_location,
                              current_uniform->data_function(component));
    }
    // Draw component.
    // Bind VAO.
    VAO * vao = component->vao;
    glBindVertexArray(vao->vao);
    // Draw elements.
    glDrawArrays(vao->vbo.render_geometry, vao->start, vao->count);
}
