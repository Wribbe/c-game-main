#include <glad/glad.h>

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

