#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "linmath.h"

#include "events/events.h"
#include "graphics/graphics.h"
#include "utils/utils.h"
#include "SOIL/SOIL.h"
#include "components/components.h"

GLfloat global_time = 1.0f;

void init(void)
{
    // Set background color.
    glClearColor( 0.55f, 0.55f, 0.55f, 0.0f);
}

enum uniform_type {
    /* Strip the gl part of the glUniform* functions used. */
    UniformMatrix4fv,
    Uniform1f,
    NUM_TYPES,
};

struct uniform_data {
    const char * name;
    void * (*data_function)(struct component *);
    enum uniform_type type;
};


void * uniform_data_transform(struct component * component)
    /* Get and return pointer to component transformation matrix. */
{
    return (void * )&component->transformation[0][0];
}

void * uniform_data_time(struct component * component)
    /* Get and return pointer to component transformation matrix. */
{
    UNUSED(component);
    return (void * )&global_time;
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


void display(
             struct component * component,
             GLuint * shader_programs,
             GLuint * textures
            )
    /* Main display function. */
{
    // Clear screen.
    glClear(GL_COLOR_BUFFER_BIT);

    struct uniform_data standard_uniforms[] = {
        {"transform", uniform_data_transform, UniformMatrix4fv},
    };
    struct component * compp;
    for(compp = components; compp != NULL; compp = compp->next) {
        // Don't draw the controlled component here.
        if (compp == controlled_component) {
            continue;
        }
        draw_component(compp,
                       shader_programs[0],
                       textures[0],
                       standard_uniforms,
                       SIZE(standard_uniforms));
    }
    struct uniform_data outline_uniforms[] = {
        {"transform", uniform_data_transform, UniformMatrix4fv},
        {"time", uniform_data_time, Uniform1f},
    };
    // Draw controlled object with other shader but same texture.
    draw_component(controlled_component,
                   shader_programs[1],
                   textures[0],
                   outline_uniforms,
                   SIZE(outline_uniforms));

    // Unbind VAO.
    glBindVertexArray(0);
    // Unbind program.
    glUseProgram(0);
}

int main(void)
{
    GLFWwindow * window;
    const char * filename;

    // Init GLFW.
    if (!glfwInit()) {
        return EXIT_FAILURE;
    }

    /* OpenGL window context hints. */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(1000, 1000, "Recreated Boing", NULL, NULL);
    if (!window) {
        fprintf(stderr, "[!] Could not create window, aborting.\n");
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    // Set key callback function for window.
    glfwSetKeyCallback(window, callback_key);

    // Initialize values.
    init();

    // Set up shaders.
    GLuint vertex_shader = 0;
    GLuint fragment_shader = 0;

    create_shader(&vertex_shader, shader_src("boing.vert"));
    create_shader(&fragment_shader, shader_src("boing.frag"));

    // Create and link program.
    GLuint shaders[] = {
        vertex_shader,
        fragment_shader,
    };

    // Set up and link shader program.
    GLuint shader_program = 0;
    link_program(&shader_program, shaders, SIZE(shaders));

    // Set up controllable shader variables.
    GLuint controllable_vertex = 0;
    GLuint controllable_fragment = 0;

    // Create shaders from text source.
    create_shader(&controllable_vertex, shader_src("controllable.vert"));
    create_shader(&controllable_fragment, shader_src("controllable.frag"));

    // Set up shader list.
    GLuint controllable_shaders[] = {
        controllable_vertex,
        controllable_fragment,
    };

    // Create and link a new shader program.
    GLuint controllable_shader_program = 0;
    link_program(&controllable_shader_program,
                 controllable_shaders,
                 SIZE(controllable_shaders));

    // Gather data.
    Point_Data point_data = {0};
    // Set up filename.
    filename = data_src("test_rectangle_rainbow.txt");
    // Run load vertex data without buffer to get dimensions.
    load_data(&point_data, NULL, filename);
    // Allocate memory for vertex data.
    float * vertex_buffer = malloc(sizeof(float) * point_data.elements);
    // Run load vertex with buffer to process the vertices.
    load_data(&point_data, vertex_buffer, filename);

    // Define VAO and VBO for vao and vbo.
    VAO vao = {0};
    VBO vbo = {0};

    // Generate buffers.
    gen_buffers(1, &vbo, &point_data, GL_STATIC_DRAW);

    // Set render type for vbo.
    vbo.render_geometry = GL_TRIANGLES;

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
        &vbo,
    };

    // Generate vertex array buffer.
    gen_vertex_arrays(
                      1,
                      &vao,
                      vbo_binds,
                      SIZE(vbo_binds),
                      attribs,
                      SIZE(attribs)
                     );

    // Set vbo as vao.vbo.
    vao.vbo = vbo;
    // Set start.
    vao.start = 0;
    // Set count.
    vao.count = vao.vbo.point_data->rows;

    // Get texture file source.
    filename = texture_src("Dietrich.jpg");

    // Set up main component.
    struct component * main_component = create_component("Dietrich",
                                                         &vao,
                                                         NULL);

    // set up second component.
    struct component * second_component = create_component("Dietrich",
                                                           &vao,
                                                           NULL);
    // Scale the dimensions.
    m4_scale(main_component->transformation, 0.4, 0.3, 0.3);
    m4_scale(second_component->transformation, 0.4, 0.3, 0.3);

    // Set up component list.
    controlled_component = main_component;
    components = main_component;
    last_component = main_component;

    // Link main and second.
    last_component->next = second_component;
    last_component = last_component->next;

    // Generate texture and load image data.
    GLuint texture;
    glGenTextures(1, &texture);
    load_to_texture(&texture, filename);

    // Setup environment variables.
    setup_globals();

    // Set up arrays of textures and shader programs.
    GLuint shader_programs[] = {
        shader_program,
        controllable_shader_program,
    };

    GLuint textures[] = {
        texture,
    };

    // Enable alpha blending.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while(!glfwWindowShouldClose(window)) {
        global_time = glfwGetTime();
        poll_events(window);
        display(components, shader_programs, textures);
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    free(vertex_buffer);
    free_component(main_component);
    free_component(second_component);
}
