#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "linmath.h"

void init(void) {
    // Set background color.
    glClearColor( 0.55f, 0.55f, 0.55f, 0.0f);
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT);
}

int main(void) {

    GLFWwindow * window;

    // Init GLFW.
    if (!glfwInit()) {
        return EXIT_FAILURE;
    }

    window = glfwCreateWindow(400, 400, "Recreated Boing", NULL, NULL);
    if (!window) {
        fprintf(stderr, "[!] Could not create window, aborting.\n");
        return EXIT_FAILURE;
    }

    glfwSetWindowAspectRatio(window, 1, 1);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    // Initialize values.
    init();

    while(!glfwWindowShouldClose(window)) {

        display();
        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    return EXIT_SUCCESS;
}
