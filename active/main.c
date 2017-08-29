#include <stdio.h>
#include <stdlib.h>

#include <GLFW/glfw3.h>

int
main(void)
{
    GLFWwindow * window;

    if (!glfwInit()) {
        return EXIT_FAILURE;
    }

    window = glfwCreateWindow(640, 480, "HELLO WORLD", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    /* Make window context current one. */
    glfwMakeContextCurrent(window);

    /* Loop until the user closes window. */
    while (!glfwWindowShouldClose(window)) {

        /* Render. */
        glClear(GL_COLOR_BUFFER_BIT);

        /* Swap. */
        glfwSwapBuffers(window);

        /* Poll events. */
        glfwPollEvents();
    }
}
