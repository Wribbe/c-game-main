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

const GLchar * source_shader_vertex =
"#version 330 core\n"
"\n"
"layout (location=0) in vec3 vertex_data;\n"
"\n"
"void main() {\n"
"  gl_Position = vec4(vertex_data, 1.0f);\n"
"}\n";

const GLchar * source_shader_fragment =
"#version 330 core\n"
"\n"
"void main() {\n"
"  gl_FragColor = vec4(1.0f);\n"
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

GLfloat vertices[] = {
    -0.5f,  0.5f, 0.0f, // Top left.
     0.5f,  0.5f, 0.0f, // Top right.
    -0.5f, -0.5f, 0.0f, // Bottom left.
     0.5f, -0.5f, 0.0f, // Bottom right.
};

GLubyte indices[] = {
    // First triangle.
    0, // Top left.
    2, // Bottom left.
    1, // Top right.
    // Second triangle.
    1, // Top right.
    2, // Bottom left.
    3, // Bottom right.
};

GLchar *
read_file(const GLchar * filename)
{
    FILE * handle_file = fopen(filename, "r");

    if (handle_file == NULL) {
        return NULL;
    }

    /* Figure out data size. */
    fseek(handle_file, 0, SEEK_END);
    size_t size_data = ftell(handle_file);
    rewind(handle_file);

    /* Allocate enough memory to store data. */
    GLchar * buffer = malloc(size_data + 1);

    /* Read the data from file. */
    size_t size_read_data = fread(buffer, 1, size_data, handle_file);

    /* Null-terminate the data. */
    buffer[size_data] = '\0';

    fclose(handle_file);
    return buffer;
}

void
obj_parse_data(const GLchar * data, GLsizei * num_vertices, GLfloat * vertices,
        GLsizei * num_indices, GLfloat * indices)
{
    const GLchar * current = data;
    const GLchar * newline = data;

    while(*current != '\0') {
        while(*newline != '\n') {
            newline++;
        }
        // Do some parsing.
        printf("NEWLINE!\n");
        printf("%.*s\n", (int)(newline-current), current);

        // Increment newline and assign current to new start.
        newline++;
        current = newline;
    }
}

int
main(void)
{
    GLFWwindow * window = setup_glfw();

    GLuint program_shader = setup_shaders();
    glUseProgram(program_shader);

    GLuint vbo = 0;
    GLuint vao = 0;

    /* Setup vertex array object. */
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    /* Setup vertex buffer object. */
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    /* Populate buffer with data. */
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    /* Setup vertex attribute pointer. */
    GLuint attribute_vertex_data = 0;
    glVertexAttribPointer(
            attribute_vertex_data, // Attribute index.
            3,          // Number of elements per vertex.
            GL_FLOAT,   // Data type of element.
            GL_FALSE,   // Specify if data is normalized.
            0,          // Stride = 0 -> elements tightly packed.
            0           // Offset-pointer to first element.
    );
    /* Enable vertex attribute pointer. */
    glEnableVertexAttribArray(attribute_vertex_data);

    const GLchar * filename = "suzanne.obj";
    GLchar * str_suzanne = read_file(filename);

    GLfloat * suzanne_vertices = NULL;
    GLsizei num_suzanne_vertices = 0;
    GLfloat * suzanne_indices = NULL;
    GLsizei num_suzanne_indices = 0;

    obj_parse_data(str_suzanne, &num_suzanne_vertices, suzanne_vertices,
            &num_suzanne_indices, suzanne_indices);

    while (!glfwWindowShouldClose(window)) {

        /* Clear color and depth buffers. */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* Poll events. */
        glfwPollEvents();

        /* Draw. */
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);

        /* Swap. */
        glfwSwapBuffers(window);

    }
    glfwTerminate();
}
