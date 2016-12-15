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
#include "globals/globals.h"

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
    for(compp = component; compp != NULL; compp = compp->next) {
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

    // Draw controlled object after other objects, to make it appear on top.
    draw_component(controlled_component,
                   shader_programs[0],
                   textures[0],
                   standard_uniforms,
                   SIZE(standard_uniforms));

    struct uniform_data outline_uniforms[] = {
        {"transform", uniform_data_transform, UniformMatrix4fv},
        {"time", uniform_data_time, Uniform1f},
    };
    // Draw controlled object with other shader but same texture for outline.
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
    global_init();

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
    set_as_controlled(main_component);
    append_component(main_component, CONTROLLABLE);

    // Link main and second.
    append_component(second_component, CONTROLLABLE);

    // Generate texture and load image data.
    GLuint texture;
    glGenTextures(1, &texture);
    load_to_texture(&texture, filename);


    // Set up arrays of textures and shader programs.
    GLuint shader_programs[] = {
        shader_program,
        controllable_shader_program,
    };

    GLuint textures[] = {
        texture,
    };

    // Setup environment variables.
    setup_globals();

    // Enable alpha blending.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while(!glfwWindowShouldClose(window)) {
        global_variables[glfw_time] = (float)glfwGetTime();
        poll_events(window);
        display(get_component(CONTROLLABLE), shader_programs, textures);
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    free(vertex_buffer);
    free_component(main_component);
    free_component(second_component);
}
