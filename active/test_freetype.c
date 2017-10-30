#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gl3w.h"
#include <GLFW/glfw3.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#define M_PI 3.14159265358979323846
#define UNUSED(x) (void)x
#define SIZE(x) sizeof(x)/sizeof(x[0])

GLuint WIDTH = 1024;
GLuint HEIGHT = 576;
size_t BYTE_DEPTH = 4;

void
die(const GLchar * message)
{
    fprintf(stderr, "[!]: %s, aborting.\n", message);
    exit(EXIT_FAILURE);
}

static void
key_callback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
    UNUSED(scancode);
    UNUSED(mods);

    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }
}

GLubyte * texture_data = NULL;

GLboolean activity_mouse_buttons[4] = {0};

static void
cursor_position_callback(GLFWwindow * window, double xpos, double ypos)
{
    UNUSED(window);

    if (activity_mouse_buttons[0]) {
        GLuint row = HEIGHT - (GLuint)ypos - 1;
        GLuint col = (GLuint)xpos;
        if (col > WIDTH - 1) {
            return;
        }
        if (row > HEIGHT - 1) {
            return;
        }

        size_t bytes_row = WIDTH * BYTE_DEPTH;
        size_t offset_row = row * bytes_row;
        size_t offset_col = col * BYTE_DEPTH;
        size_t index = offset_row+offset_col;

        GLubyte * color_pointer = &texture_data[index];
        /* Set RGB to 0. */
        *color_pointer++ = 0;
        *color_pointer++ = 0;
        *color_pointer++ = 0;
    }
}

static void
mouse_button_callback(GLFWwindow * window, int button, int action, int mods)
{
    UNUSED(window);
    UNUSED(mods);
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            activity_mouse_buttons[0] = GL_TRUE;
        } else {
            activity_mouse_buttons[0] = GL_FALSE;
        }
    }
}

GLfloat vertices_rectangle[] = {
    // First triangle.
    -1.0f,  1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
    // Second triangle.
    -1.0f,  1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
     1.0f,  1.0f, 0.0f,
};

const GLchar * source_fragment = \
"#version 330 core\n"
"\n"
"in vec2 UV;\n"
"\n"
"uniform sampler2D sampler_texture;\n"
"\n"
"void main() {\n"
"   gl_FragColor = texture(sampler_texture, UV);\n"
"   //gl_FragColor = vec4(UV.x, UV.y, 0.0f, 1.0f);\n"
"   //vec4 data_texture = texture(sampler_texture, UV);\n"
"   //gl_FragColor = vec4(data_texture.r, 1.0f, 1.0f, 1.0f);\n"
"}\n";

const GLchar * source_vertex = \
"#version 330 core\n"
"layout (location = 0) in vec3 vPosition;\n"
"layout (location = 1) in vec2 vUV;\n"
"\n"
"out vec2 UV;\n"
"\n"
"void main() {\n"
"   gl_Position = vec4(vPosition, 1.0f);\n"
"   UV = vUV;\n"
"}\n";

GLint
compile_shader(GLuint id_shader)
{
    glCompileShader(id_shader);
    GLint success = 0;
    glGetShaderiv(id_shader, GL_COMPILE_STATUS, &success);
    return success;
}

void
print_shader_error(GLuint id_shader) {
    size_t size_buffer = 1024;
    char buffer_log[size_buffer];
    glGetShaderInfoLog(id_shader, size_buffer, NULL, buffer_log);
    fprintf(stderr, "Compilation of shader failed: %s\n", buffer_log);
}

void
print_program_error(GLuint id_program) {
    size_t size_buffer = 1024;
    char buffer_log[size_buffer];
    glGetProgramInfoLog(id_program, size_buffer, NULL, buffer_log);
    fprintf(stderr, "Compilation of shader failed: %s\n", buffer_log);
}

GLint
assemble_program(GLuint id_program, const GLchar * const source_vertex,
        const GLchar * const source_fragment)
{
    /* Create shaders. */
    GLuint shader_vertex = glCreateShader(GL_VERTEX_SHADER);
    GLuint shader_fragment = glCreateShader(GL_FRAGMENT_SHADER);

    /* Set sources. */
    glShaderSource(shader_vertex, 1, &source_vertex, NULL);
    glShaderSource(shader_fragment, 1, &source_fragment, NULL);

    /* Compile shaders. */
    if (compile_shader(shader_vertex) != GL_TRUE) {
        print_shader_error(shader_vertex);
        exit(EXIT_FAILURE);
    }

    if (compile_shader(shader_fragment) != GL_TRUE) {
        print_shader_error(shader_fragment);
        exit(EXIT_FAILURE);
    }

    glAttachShader(id_program, shader_vertex);
    glAttachShader(id_program, shader_fragment);
    glLinkProgram(id_program);
    GLint success = 0;
    glGetProgramiv(id_program, GL_LINK_STATUS, &success);

    /* Delete shaders. */
    glDeleteShader(shader_vertex);
    glDeleteShader(shader_fragment);

    return success;
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

    window = glfwCreateWindow(WIDTH, HEIGHT, "HELLO WORLD", NULL, NULL);
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

    /* Set key callback function for main window. */
    glfwSetKeyCallback(window, key_callback);
    /* Set mouse position callback function for main window. */
    glfwSetCursorPosCallback(window, cursor_position_callback);
    /* Set mouse button callback function for main window. */
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    /*  Create shader program for map. */
    GLuint basic_program = glCreateProgram();
    if (assemble_program(basic_program, source_vertex, source_fragment) != GL_TRUE) {
        print_program_error(basic_program);
        return EXIT_FAILURE;
    }

    /* Set up OpenGL buffers. */
    GLuint VBO = 0;
    GLuint VAO = 0;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);

    /* Bind vertex array and buffer. */
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    /* Populate VBO with data. */
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_rectangle)*sizeof(GLfloat),
            vertices_rectangle, GL_STATIC_DRAW);

    /* Set and enable correct vertex attribute for vertex position (0). */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    /* Set up UV buffer. */
    GLuint buffer_uv = 0;
    glGenBuffers(1, &buffer_uv);
    glBindBuffer(GL_ARRAY_BUFFER, buffer_uv);

    GLfloat coordinates_UV[] = {
        // First triangle.
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        // Second triangle.
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(coordinates_UV), coordinates_UV, GL_STATIC_DRAW);

    /* Set and enable correct vertex attribute for UV coordinates (1). */
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);

    glUseProgram(basic_program);

    /* Create OpenGL texture. */
    GLuint id_texture;
    glGenTextures(1, &id_texture);

    size_t tex_width = WIDTH;
    size_t tex_height = HEIGHT;
    size_t tex_bits = 4;
    GLubyte local_data[tex_height][tex_width][tex_bits];
    texture_data = &local_data[0][0][0];
    memset(texture_data, (int)(pow(2,8)-1-72), tex_width*tex_height*tex_bits);
    memset(texture_data, (int)(pow(2,8)-1), tex_width*(tex_height/2)*tex_bits);

    /* Bind texture. */
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id_texture);

    /* Set sampler to 0. */
    GLuint location_sampler_texture = glGetUniformLocation(basic_program,
            "sampler_texture");
    glUniform1i(location_sampler_texture, 0);

    /* Give texture data to OpenGL. */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width, tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);

//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glfwSwapInterval(0);

    while (!glfwWindowShouldClose(window)) {

        /* Render. */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* Poll events. */
        glfwPollEvents();

        /* Reload texture data. */
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width, tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);

        /* Draw. */
        glDrawArrays(GL_TRIANGLES, 0, SIZE(vertices_rectangle)/3);

        /* Swap. */
        glfwSwapBuffers(window);

    }

    glfwTerminate();
}
