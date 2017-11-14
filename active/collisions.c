#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gl3w.h"
#include <GLFW/glfw3.h>

#include "linmath.h"

#define M_PI 3.14159265358979323846
#define UNUSED(x) (void)x
#define SIZE(x) sizeof(x)/sizeof(x[0])

#define WINDOW_WIDTH 1440
#define WINDOW_HEIGHT 900

GLFWwindow *
setup_glfw(void)
{
    GLFWwindow * window;

    if (!glfwInit()) {
        fprintf(stderr, "Could not initialize glfw, aborting.\n");
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "HELLO WORLD", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Could not create window, aborting.\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    /* Make window context current one. */
    glfwMakeContextCurrent(window);

    if (gl3wInit()) {
        fprintf(stderr, "Could not initialize gl3w, aborting.\n");
        exit(EXIT_FAILURE);
    }

    if (!gl3wIsSupported(3, 3)) {
        fprintf(stderr, "Profile 3.3 not supported, aborting.\n");
        exit(EXIT_FAILURE);
    }

    printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION),
            glGetString(GL_SHADING_LANGUAGE_VERSION));

    return window;
}

int
main(void)
{
    GLFWwindow * window = setup_glfw();

    while (!glfwWindowShouldClose(window)) {

        /* Clear color and depth buffers. */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* Poll events. */
        glfwPollEvents();

        /* Swap. */
        glfwSwapBuffers(window);

    }
    glfwTerminate();
}
