#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "gl3w.h"
#include "linmath.h"
#include <GLFW/glfw3.h>

#define M_PI 3.14159265358979323846

GLchar *
read_file(const char * filepath)
{
    FILE * fpointer =  fopen(filepath, "r");
    if (fpointer == NULL) {
        fprintf(stderr, "Could not find file %s, aborting.\n", filepath);
        exit(EXIT_FAILURE);
    }

    fseek(fpointer, 0, SEEK_END);
    size_t size = ftell(fpointer);
    rewind(fpointer);

    GLchar * data = malloc(size+1);
    if (data == NULL) {
        fprintf(stderr, "Could not allocate memory for reading file,"
                " aborting.\n");
        exit(EXIT_FAILURE);
    }

    fread(data, size, 1, fpointer);
    data[size] = '\0';

    return data;
}

GLuint
create_shader_program(const char * vertex_source_path,
                      const char * fragment_source_path)
{
    GLchar * source_vertex = read_file(vertex_source_path);
    GLchar * source_fragment = read_file(fragment_source_path);

    GLuint shader_vert = glCreateShader(GL_VERTEX_SHADER);
    GLuint shader_frag = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(shader_vert, 1, (const GLchar **)&source_vertex, NULL);
    glShaderSource(shader_frag, 1, (const GLchar **)&source_fragment, NULL);

    size_t size_buffer = 1024;
    char buffer_log[size_buffer];
    GLint success = 0;

    glCompileShader(shader_vert);
    glGetShaderiv(shader_vert, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        glGetShaderInfoLog(shader_vert, size_buffer, NULL, buffer_log);
        fprintf(stderr,
                "Compilation of vertex shader failed, aborting: \n%s\n",
                buffer_log);
        fprintf(stderr, "Shader data:\n\n%s\n", source_vertex);
        exit(EXIT_FAILURE);
    }

    glCompileShader(shader_frag);
    glGetShaderiv(shader_frag, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        glGetShaderInfoLog(shader_frag, size_buffer, NULL, buffer_log);
        fprintf(stderr,
                "Compilation of fragment shader failed, aborting: \n%s\n",
                buffer_log);
        fprintf(stderr, "Shader data:\n\n%s\n", source_fragment);
        exit(EXIT_FAILURE);
    }

    GLuint program = glCreateProgram();

    glAttachShader(program, shader_vert);
    glAttachShader(program, shader_frag);

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (success != GL_TRUE) {
        glGetProgramInfoLog(program, size_buffer, NULL, buffer_log);
        fprintf(stderr,
                "Linking of program failed, aborting: \n%s\n",
                buffer_log);
        exit(EXIT_FAILURE);
    }

    glDeleteShader(shader_vert);
    glDeleteShader(shader_frag);

    free(source_vertex);
    free(source_fragment);

    return program;
}

size_t
get_data(const char * filepath, GLfloat ** float_data)
{
    char * data = read_file(filepath);
    size_t num_values = 0;
    char * pointer = data;
    for (;*pointer != '\0'; pointer++) {
        if (*pointer == ',') {
            num_values++;
        }
    }

    *float_data = malloc(num_values*sizeof(GLfloat));
    if (*float_data == NULL) {
        fprintf(stderr,
                "Could not allocate enough memory for float data, "
                "aborting.\n");
        exit(EXIT_FAILURE);
    }

    char * data_pointer = data;
    float * current_float = *float_data;

    for(;;data_pointer++) {
        char c = *data_pointer;
        if (c == '\0') {
            break;
        }
        if(c != '-' && c != '+' && !isdigit(c)) {
            continue;
        }
        *current_float++ = strtof(data_pointer, &data_pointer);
    }

    free(data);

    return num_values;
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

    GLuint WIDTH = 640;
    GLuint HEIGHT = 480;

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

    GLuint program = create_shader_program("vert_simple.glsl",
                                           "frag_simple.glsl");
    glUseProgram(program);
    GLfloat * data_elements = NULL;
    size_t num_elements = get_data("cube.dat", &data_elements);

    GLuint array_buffer;
    glGenVertexArrays(1, &array_buffer);
    glBindVertexArray(array_buffer);

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, num_elements*sizeof(GLfloat), data_elements,
            GL_STATIC_DRAW);

    GLuint vPosition = 0;
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(vPosition);

    // Projection matrix; FOV: PI/4, RATIO: 4:3 , RANGE: 0.1 -> 100 units.
    mat4x4 m4_projection = {0};
    mat4x4_perspective(m4_projection, M_PI/4, (float)WIDTH/(float)HEIGHT, 0.1f,
            100.f);

    // Camera matrix.
    mat4x4 m4_view = {0};
    mat4x4_look_at(m4_view, (vec3){4,3,3}, (vec3){0,0,0}, (vec3){0,1,0});

    // Model matrix.
    mat4x4 m4_model = {0};
    mat4x4_identity(m4_model);

    // MVP matrix.
    mat4x4 m4_mvp = {0};
    mat4x4_mul(m4_mvp, m4_view, m4_model);
    mat4x4_mul(m4_mvp, m4_projection, m4_mvp);

    GLuint uniform_mvp = glGetUniformLocation(program, "mvp");
    glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, &m4_mvp[0][0]);

    /* Loop until the user closes window. */
    while (!glfwWindowShouldClose(window)) {

        /* Render. */
        glClear(GL_COLOR_BUFFER_BIT);

        /* Draw. */
        glDrawArrays(GL_TRIANGLES, 0, num_elements/3);

        /* Swap. */
        glfwSwapBuffers(window);

        /* Poll events. */
        glfwPollEvents();
    }

    glfwTerminate();
    free(data_elements);

    return EXIT_SUCCESS;
}
