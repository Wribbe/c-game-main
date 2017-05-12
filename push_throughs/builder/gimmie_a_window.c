#include <stdio.h>
#include <stdlib.h>

#include "glfw3.h"

int main(void)
{
    GLFWwindow * window;

    if (!glfwInit()) {
        fprintf(stderr, "could not initiate glfw..\n");
        return EXIT_FAILURE;
    }

    window = glfwCreateWindow(640, 480, "HELLO WORLD", NULL, NULL);
    if (!window) {
        glfwTerminate();
        fprintf(stderr, "could not initiate window..\n");
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);

    while(!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
