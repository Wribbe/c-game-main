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

GLuint shader_program = 0;

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shader_program);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
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

    // Initialize memory structures.
    init_memory();

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

    link_program(&shader_program, shaders, SIZE(shaders));

    // Gather data.
    Point_Data info = {0};
    const char * filename = data_src("test_triangle.txt");
    load_data(&info, NULL, filename);
    float buffer[info.elements];
    load_data(&info, buffer, filename);

    for (int i = 0; i<info.elements; i++) {
        printf("data[%d]: %f\n", i, buffer[i]);
    }

    while(!glfwWindowShouldClose(window)) {

        display();
        glfwSwapBuffers(window);
        glfwPollEvents();

    }
}
