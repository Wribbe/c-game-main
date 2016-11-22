#include <glad/glad.h>
#include <graphics/graphics.h>

#include <stdlib.h>
#include <stdio.h>
#include "utils/utils.h"

float temp_points[] = {
    0.0f,  0.5f,  0.0f,
    0.5f, -0.5f,  0.0f,
   -0.5f, -0.5f,  0.0f,
};
float * points = temp_points;

GLuint vbo = 0;
GLuint vao = 0;

#define shader_src(x) "src/glsl/"x

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

GLuint vertex_shader = 0;
GLuint fragment_shader = 0;


void create_shader(GLuint * shader, GLuint type,  const char * source_filename)
{
    /* Create and return shader based on contents of file. */

    *shader = glCreateShader(type);

    // Open and read size of file.
    long filesize = 0;
    FILE * filehandle = open_file(source_filename, &filesize);
    if (!filehandle) {
        fprintf(stderr, "[!] create_shader: Could not find shader source file: %s\n",
                source_filename);
        exit(1);
    }

    char temp_buffer[filesize];
    char * string_p = temp_buffer;

    // Read data to temp_buffer.
    size_t read = read_file(temp_buffer, filesize, filehandle);
    if (!read) {
        fprintf(stderr, "[!] create_shader: No objects read from %s\n",
                source_filename);
        exit(1);
    }

    glShaderSource(*shader, 1, &string_p, NULL);
    glCompileShader(*shader);
    checkShaderStatus(*shader);
}


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
    create_shader(&vertex_shader, GL_VERTEX_SHADER, shader_src("boing.vert"));
    checkShaderStatus(vertex_shader);

    // Set up fragment shader.
    create_shader(&fragment_shader, GL_FRAGMENT_SHADER, shader_src("boing.frag"));
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

