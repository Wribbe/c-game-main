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

GLuint WINDOW_WIDTH = 1440;
GLuint WINDOW_HEIGHT = 900;

GLboolean key_down[500] = {0};

GLFWwindow * current_window = NULL;
GLboolean updated_keys = GL_FALSE;

static void
keyboard_key_callback(GLFWwindow * window, int key, int scancode, int action,
        int mods)
{
    UNUSED(mods);
    UNUSED(scancode);
    UNUSED(window);
    if (action == GLFW_PRESS) {
        key_down[key] = GL_TRUE;
    } else {
        key_down[key] = GL_FALSE;
    }
    updated_keys = GL_TRUE;
}

void
process_events(void)
{
    if (!updated_keys) {
        return;
    }
    updated_keys = GL_FALSE;

    if (key_down[GLFW_KEY_ESCAPE]) {
        glfwSetWindowShouldClose(current_window, GLFW_TRUE);
    }
}

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

    glfwSetKeyCallback(window, keyboard_key_callback);
    current_window = window;

    return window;
}

struct draw_object {
    GLsizei num_vertices;
    GLfloat * points;
};

GLfloat vertices_triangle[] = {
    -0.5f, -0.5f,  0.0f,
     0.5f, -0.5f,  0.0f,
     0.0f,  0.5f,  0.0f
};

struct draw_object triangle = {
    SIZE(vertices_triangle),
    vertices_triangle,
};

void
draw_objects()
{
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

GLuint
setup_buffers()
{
    GLuint vbo = 0; // Vertex buffer object.
    GLuint vao = 0; // Vertex array object.

    /* Generate vertex array object name. */
    glGenVertexArrays(1, &vao);
    /* Bind vertex array object. */
    glBindVertexArray(vao);

    /* Generate vertex buffer object name. */
    glGenBuffers(1, &vbo);
    /* Bind vertex buffer object as array buffer. */
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    /* Populate buffer with data. */
    glBufferData(GL_ARRAY_BUFFER, triangle.num_vertices*sizeof(GLfloat),
            triangle.points, GL_STATIC_DRAW);

    /* Setup and enable vertex data attribute pointer. */
    GLuint attribute_vertex_data = 0;
    glEnableVertexAttribArray(attribute_vertex_data);
    glVertexAttribPointer(
            attribute_vertex_data, // Target to enable.
            3,         // Number of elements per 'chunk'.
            GL_FLOAT,  // Size of each element.
            GL_FALSE,  // Should openGL normalize the elements?
            0,         // Stride, is there an offset between chunks?
            (void*)0   // Pointer to first set of vertex data.
    );

    /* Un-bind the vertex buffer and vertex array objects. */
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return vao;
}

const GLchar * source_shader_vertex =
"#version 330 core\n"
"\n"
"layout (location=0) in vec3 vertex_data;\n"
"\n"
"void main() {\n"
"\n"
"  gl_Position = vec4(vertex_data, 1.0f);\n"
"\n"
"}\n";

const GLchar * source_shader_fragment =
"#version 330 core\n"
"out vec4 FragColor;\n"
"\n"
"void main() {\n"
"  FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
"}\n";

GLuint
setup_shaders()
{
    /* Create vertex and fragment shaders. */
    GLuint shader_vertex = glCreateShader(GL_VERTEX_SHADER);
    GLuint shader_fragment = glCreateShader(GL_FRAGMENT_SHADER);

    /* Load correct source into each shader. */
    glShaderSource(shader_vertex, 1, &source_shader_vertex, NULL);
    glShaderSource(shader_fragment, 1, &source_shader_fragment, NULL);

    /* Compile shaders. */
    glCompileShader(shader_vertex);
    glCompileShader(shader_fragment);

    /* Prepare error data structures. */
    GLint operation_successful = 0;
    GLsizei size_error_buffer = 1024;
    GLchar buffer_error_message[size_error_buffer];

    /* Check compilation status of vertex shader. */
    glGetShaderiv(shader_vertex, GL_COMPILE_STATUS, &operation_successful);
    if (!operation_successful) {
        glGetShaderInfoLog(shader_vertex, size_error_buffer, NULL,
                buffer_error_message);
        fprintf(stderr, "Error on compiling vertex shader: %s\n\n%s\n",
                buffer_error_message, source_shader_vertex);
    }

    /* Check compilation status of fragment shader. */
    glGetShaderiv(shader_fragment, GL_COMPILE_STATUS, &operation_successful);
    if (!operation_successful) {
        glGetShaderInfoLog(shader_fragment, size_error_buffer, NULL,
                buffer_error_message);
        fprintf(stderr, "Error on compiling fragment shader: %s\n\n%s\n",
                buffer_error_message, source_shader_fragment);
    }

    /* Create shader program. */
    GLuint program_shader = glCreateProgram();

    /* Attach compiled shaders. */
    glAttachShader(program_shader, shader_vertex);
    glAttachShader(program_shader, shader_fragment);

    /* Link the shader program. */
    glLinkProgram(program_shader);

    /* Check link status. */
    glGetProgramiv(program_shader, GL_LINK_STATUS, &operation_successful);
    if (!operation_successful) {
        glGetProgramInfoLog(program_shader, size_error_buffer, NULL,
                buffer_error_message);
        fprintf(stderr, "Error on linking the shader program: %s\n",
                buffer_error_message);
    }

    /* Delete compiled shaders. */
    glDeleteShader(shader_vertex);
    glDeleteShader(shader_fragment);

    return program_shader;
}


int
main(void)
{
    GLFWwindow * window = setup_glfw();

    GLuint id_vao = setup_buffers();
    GLuint id_program = setup_shaders();

    glBindVertexArray(id_vao);
    glUseProgram(id_program);

    while (!glfwWindowShouldClose(window)) {

        /* Clear color and depth buffers. */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* Poll events. */
        glfwPollEvents();
        process_events();

        /* Draw */
        draw_objects();

        /* Swap. */
        glfwSwapBuffers(window);

    }
}
