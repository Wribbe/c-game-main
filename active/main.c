#include <stdio.h>
#include <stdlib.h>

#include "gl3w.h"
#include <GLFW/glfw3.h>

char *
read_file(const char * filepath)
{
    FILE * fpointer =  fopen(filepath, "r");
    if (fpointer == NULL) {
        fprintf(stderr, "Could not find file %s, aborting.\n", filepath);
        exit(EXIT_FAILURE);
    }

    fseek(fpointer, 0, SEEK_END);
    size_t size = ftell(fpointer)+1;
    rewind(fpointer);

    char * data = malloc(size);
    if (data == NULL) {
        fprintf(stderr, "Could not allocate memory for reading file,"
                " aborting.\n");
        exit(EXIT_FAILURE);
    }

    fread(data, size, 1, fpointer);
    data[size-1] = '\0';

    return data;
}

int
main(void)
{
    GLFWwindow * window;

    if (!glfwInit()) {
        fprintf(stderr, "Could not initialize glfw, aborting.\n");
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(640, 480, "HELLO WORLD", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Could not create window, aborting.\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    /* Make window context current one. */
    glfwMakeContextCurrent(window);

    if (gl3wInit()) {
        fprintf(stderr, "Could not initialize gl3w, aborting.\n");
        return EXIT_FAILURE;
    }

    if (!gl3wIsSupported(3, 3)) {
        fprintf(stderr, "Profile 3.3 not supported, aborting.\n");
        return EXIT_FAILURE;
    }

    printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION),
            glGetString(GL_SHADING_LANGUAGE_VERSION));

    char * test = read_file("vert_simple.glsl");
    printf("test: %s\n", test);

    /* Loop until the user closes window. */
    while (!glfwWindowShouldClose(window)) {

        /* Render. */
        glClear(GL_COLOR_BUFFER_BIT);

        /* Swap. */
        glfwSwapBuffers(window);

        /* Poll events. */
        glfwPollEvents();
    }

    glfwTerminate();
    free(test);

    return EXIT_SUCCESS;
}
