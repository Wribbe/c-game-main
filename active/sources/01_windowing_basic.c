#include <stdlib.h>
#include <stdio.h>

#include "glad.h"
#include <GLFW/glfw3.h>


void prefixed_output(FILE * output,
                     const char * tag,
                     const char * message)
{
    fprintf(output, "%s %s\n", tag, message);
}

void error_and_exit(const char * message)
{
    const char * tag = "[!]";
    prefixed_output(stderr, tag, message);
    exit(EXIT_FAILURE);
}

void glfw_set_context(void)
    /* OpenGL window context hints. */
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

int main(void)
{
    if (!glfwInit()) {
        error_and_exit("Could not initialize GLFW, aborting.\n");
    }

    glfw_set_context();

    GLFWwindow * window = glfwCreateWindow(800,
                                           600,
                                           "windowing_basic",
                                           NULL,
                                           NULL);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
}
