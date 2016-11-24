#include <glad/glad.h>
#include <graphics/graphics.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utils/utils.h"

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

    float * bounds = vao->bounds;
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

    // Use pointer arithmetic to write to the bounds array.
    for(size_t i=0; i<SIZE(order); i+=3) { // One row at the time.
        *(bounds++) = order[i];
        *(bounds++) = order[i+1];
        *(bounds++) = order[i+2];
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
