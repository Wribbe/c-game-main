#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "linmath.h"

#include "events/events.h"

void init(void)
{
    // Set background color.
    glClearColor( 0.55f, 0.55f, 0.55f, 0.0f);
}


float points[] = {
    0.0f,  0.5f,  0.0f,
    0.5f, -0.5f,  0.0f,
   -0.5f, -0.5f,  0.0f,
};

GLuint vbo = 0;
GLuint vao = 0;

const char * vertex_shader_source =
"#version 330\n"
"in vec3 vp;\n"
"void main() {\n"
"   gl_Position = vec4(vp, 1.0);\n"
"}\n";

const char * fragment_shader_source =
"#version 330\n"
"out vec4 frag_color;\n"
"void main() {\n"
"   frag_color = vec4(0.5f, 0.0f, 0.5f, 1.0f);\n"
"}\n";

void init_memory() {

    // Generate and populate Vertex Buffer Object.
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), points, GL_STATIC_DRAW);

    // Generate Vertex Array Object.
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

}

GLuint vertex_shader;
GLuint fragment_shader;


void checkShaderStatus(GLuint shader) {
    GLint status = 0;

    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (status == GL_TRUE) {
        return;
    }

    GLint logSize = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);

    char message_buffer[logSize+1];

    glGetShaderInfoLog(shader, logSize, NULL, message_buffer);
    printf("Error in shader compilation: %s\n", message_buffer);
}

void create_shaders() {

    // Set up vertex shader.
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);
    checkShaderStatus(vertex_shader);

    // Set up fragment shader.
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);
    checkShaderStatus(fragment_shader);

}

GLuint shader_program;

void link_program() {

   shader_program = glCreateProgram();
   glAttachShader(shader_program, vertex_shader);
   glAttachShader(shader_program, fragment_shader);
   glLinkProgram(shader_program);

}

void setup_shaders() {
    create_shaders();
    link_program();
}

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
    setup_shaders();

    while(!glfwWindowShouldClose(window)) {

        display();
        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    return EXIT_SUCCESS;
}
