#include <stdio.h>
#include <stdlib.h>

#include "GL/gl3w.h"
#include "GLFW/glfw3.h"

const char * source_frag_simple =\
    "#version 330 core\n"
    "\n"
    "out vec4 color;\n"
    "\n"
    "void main() {\n"
    "   color = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
    "}\n";

const char * source_vert_simple =\
    "#version 330 core\n"
    "\n"
    "layout (location = 0) in vec3 position;\n"
    "\n"
    "void main() {\n"
    "   gl_Position = vec4(position, 1.0f);\n"
    "}\n";

GLfloat vertices[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
};

int main(void)
{
    GLFWwindow * window;

    if (!glfwInit()) {
        fprintf(stderr, "could not initiate glfw..\n");
        return EXIT_FAILURE;
    }

    // Set context hints.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(640, 480, "HELLO WORLD", NULL, NULL);
    if (!window) {
        glfwTerminate();
        fprintf(stderr, "could not initiate window..\n");
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    if (gl3wInit()) {
        fprintf(stderr, "Failed to intialize OpenGL\n");
        return EXIT_FAILURE;
    }

    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Set up vertex attribute pointers.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // Create shader variables.
    GLuint shader_vert_simple = glCreateShader(GL_VERTEX_SHADER);
    GLuint shader_frag_simple = glCreateShader(GL_FRAGMENT_SHADER);

    // Bind sources.
    glShaderSource(shader_vert_simple, 1, &source_vert_simple, NULL);
    glShaderSource(shader_frag_simple, 1, &source_frag_simple, NULL);

    // Setup error handling for compiling.
    GLint success = 0;
    size_t size_info_log = 512;
    GLchar info_log[size_info_log];

    //Compile and check shaders.
    glCompileShader(shader_vert_simple);
    glGetShaderiv(shader_vert_simple, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(shader_vert_simple, size_info_log, NULL, info_log);
        fprintf(stderr, "Could not compile vert shader: %s\n", info_log);
    }
    glCompileShader(shader_frag_simple);
    glGetShaderiv(shader_frag_simple, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(shader_frag_simple, size_info_log, NULL, info_log);
        fprintf(stderr, "Could not compile frag shader: %s\n", info_log);
    }

    // Set up shader program.
    GLuint program_shader_simple = glCreateProgram();
    glAttachShader(program_shader_simple, shader_vert_simple);
    glAttachShader(program_shader_simple, shader_frag_simple);

    // Link program.
    glLinkProgram(program_shader_simple);

    // Check linkage.
    glGetProgramiv(program_shader_simple, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(program_shader_simple, size_info_log, NULL, info_log);
        fprintf(stderr, "Could not link program: %s\n", info_log);
    }

    // Use linked program.
    glUseProgram(program_shader_simple);

    // Delete shaders.
    glDeleteShader(shader_vert_simple);
    glDeleteShader(shader_frag_simple);

    while(!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
