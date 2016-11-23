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

void init(void)
{
    // Set background color.
    glClearColor( 0.55f, 0.55f, 0.55f, 0.0f);
}

void display(GLuint vao, GLuint shader_program)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shader_program);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

int main(void)
{
    GLFWwindow * window;

    // Init GLFW.
    if (!glfwInit()) {
        return EXIT_FAILURE;
    }

    /* OpenGL window context hints. */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(400, 400, "Recreated Boing", NULL, NULL);
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

    // Gather data.
    Point_Data point_data = {0};
    const char * filename = data_src("test_triangle.txt");
    load_data(&point_data, NULL, filename);
    float buffer[point_data.elements];
    load_data(&point_data, buffer, filename);

    // Define GLuints for vbo and vba.
    GLuint vbo;
    GLuint vao;

    // Generate buffers.
    gen_buffers(1, &vbo, &point_data, GL_STATIC_DRAW);

    // Set up attribute pointer information.
    Attrib_Pointer_Info attribs[] = {
        (Attrib_Pointer_Info){
            .index = 0,
            .size = 3,
            .stride = 0,
            .offset = NULL,
        },
    };

    // Set up the vbos that should be bound to the vao.
    GLuint vbo_binds[] = {
        vbo,
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

    filename = data_src("test_rectangle.txt");
    load_data(&point_data, NULL, filename);
    float second_buffer[point_data.elements];
    load_data(&point_data, second_buffer, filename);

    printf("elements: %d, rows: %d\n", point_data.elements, point_data.rows);

    for (int i=0; i<point_data.elements; i++) {
        printf("%f\n", second_buffer[i]);
    }

    while(!glfwWindowShouldClose(window)) {

        display(vao, shader_program);
        glfwSwapBuffers(window);
        glfwPollEvents();

    }
}
