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
obj_parse_data(const GLchar * data, GLsizei * num_vertices, GLfloat ** r_vertices,
        GLsizei * num_indices, GLuint ** r_indices)
{
    const GLchar * current = data;
    const GLchar * newline = data;

    GLsizei parsed_vertices = 0;
    GLsizei parsed_indices = 0;

    GLsizei increment_vertices = 300;
    GLsizei increment_indices = 300;

    GLsizei max_vertices = 600;
    GLsizei max_indices = 600;

    /* Allocate memory for vertices and uv coordinates. */
    GLfloat * vertices = malloc(sizeof(GLfloat)*max_vertices);
    if (vertices == NULL) {
        fprintf(stderr, "Could not allocate memory for vertices!\n");
    }
    GLuint * indices = malloc(sizeof(GLuint)*max_indices);

    size_t s_tag = 3;
    GLchar tag[s_tag];

    #define S_FORMAT 256
    GLchar format_vn[S_FORMAT] = {0};
    GLchar format_v[S_FORMAT]= {0};

    const GLchar * TAG_FACE = "f";
    const GLchar * TAG_NORMAL = "vn";
    const GLchar * TAG_VERTICE = "v";

    GLfloat f1  = 0;
    GLfloat f2  = 0;
    GLfloat f3  = 0;

    GLuint f_values[3] = {0};

    snprintf(format_v, S_FORMAT, "%%*%zus %%f %%f %%f", s_tag);

    while(*current != '\0') {
        while(*newline != '\n') {
            newline++;
        }

        /* Grab the tag. */
        sscanf(current, "%3s", tag);

        if (strcmp(tag, TAG_FACE) == 0) {
            const GLchar * start = current+2; // Skip the 't ' tag.
            GLuint * current_f_value = f_values;
            for(;;) {

                /* Going to assume that there is always a vertice index. */
                int scanned = sscanf(start, "%u", current_f_value++);
                if (scanned == 0) {
                    *(current_f_value-1) = 0; // No value was parsed.
                }

                /* Loop and parse until either a / is found triplet is ended
                 * with a space. */
                int spin = 1;
                char c;
                while(spin) {
                    c = *start;
                    switch (c) {
                        case '\n':
                        case ' ':
                        case '/':
                            spin = 0;
                        break;
                        default:
                            start++;
                        break;
                    }
                }

                /* Check for tripled and end-of-line delimiters.*/
                if (c == ' ' || c == '\n') {
                    /* Handle the parsed values and reset the current_f_value
                     * pointer. */
                    if (f_values[0] != 0) { // Vertice index was parsed.
                        indices[parsed_indices++] = f_values[0]-1;
                        /* Check if there is more room, otherwise expand. */
                        if (parsed_indices >= max_indices) {
                            max_indices += increment_indices;
                            indices = realloc(indices,
                                    sizeof(GLfloat)*max_indices);
                        }
                    }
                    if (f_values[1] != 0) { // Texture index was parsed.
//                        printf("Texture index: %u\n",  f_values[1]);
                    }
                    if (f_values[2] != 0) { // Face normal index was parsed.
//                        printf("Normal index: %u\n",  f_values[2]);
                    }
                    current_f_value = f_values;
                }

                if (c == '\n') {
                    break; // End the outer for-loop.
                }

                /* Skip any 'delimiter' that was not a newline. */
                start++;
            }
        } else if (strcmp(tag, TAG_NORMAL) == 0) {
        } else if (strcmp(tag, TAG_VERTICE) == 0) {
            /* Asterisk in format string ignores assignment. %*3s will ignore
             * strings of length up to and including 3. */
            sscanf(current, format_v, &f1, &f2, &f3);
            /* Check if there is enough room. */
            if (parsed_vertices+3 >= max_vertices) {
                max_vertices += increment_vertices;
                vertices = realloc(vertices,
                        sizeof(GLfloat)*max_vertices);
            }
            /* Put vertices in array. */
            vertices[parsed_vertices++] = f1;
            vertices[parsed_vertices++] = f2;
            vertices[parsed_vertices++] = f3;
        }


        // Increment newline and assign current to new start.
        newline++;
        current = newline;
    }

    /* Fit the vertice data. */
    if (parsed_vertices != max_vertices) {
        vertices = realloc(vertices, sizeof(GLfloat)*parsed_vertices);
    }
    /* Set the number of parsed vertices. */
    *num_vertices = parsed_vertices;

    /* Bind the vertices back to the supplied pointer. */
    *r_vertices = vertices;

    /* Set the number of parsed indices. */
    *num_indices = parsed_indices;

    /* Bind the indices back to the supplied pointer. */
    *r_indices= indices;
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

    //const GLchar * filename = "suzanne.obj";
    const GLchar * filename = "cube.obj";
    GLchar * str_obj = read_file(filename);
    if (str_obj == NULL) {
        fprintf(stderr, "Could not open %s, aborting.\n", filename);
        return EXIT_FAILURE;
    }

    GLfloat * obj_vertices = NULL;
    GLsizei num_obj_vertices = 0;
    GLuint * obj_indices = NULL;
    GLsizei num_obj_indices = 0;

    obj_parse_data(str_obj, &num_obj_vertices, &obj_vertices,
            &num_obj_indices, &obj_indices);

    /* Unbind current vertex-array and vertex-buffer object. */
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    /* Create a new vao for obj. */
    GLuint vao_obj = 0;
    glGenVertexArrays(1, &vao_obj);

    /* Bind the new vao. */
    glBindVertexArray(vao_obj);

    /* Create a buffer for obj. */
    GLuint vbo_obj = 0;
    glGenBuffers(1, &vbo_obj);

    /* Bind new buffer. */
    glBindBuffer(GL_ARRAY_BUFFER, vbo_obj);

    /* Populate buffer with data. */
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*num_obj_vertices,
            obj_vertices, GL_STATIC_DRAW);

    /* Is it necessary to setup a new vertex attribute pointer? */
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

    /* Create buffer for indices. */
    GLuint vbo_obj_indices = 0;
    glGenBuffers(1, &vbo_obj_indices);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_obj_indices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*num_obj_indices,
            obj_indices, GL_STATIC_DRAW);

    while (!glfwWindowShouldClose(window)) {

        /* Clear color and depth buffers. */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* Poll events. */
        glfwPollEvents();

        /* Draw. */
//        glBindVertexArray(vao);
//        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);

        glBindVertexArray(vao_obj);
        glDrawElements(GL_TRIANGLES, num_obj_indices, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        /* Swap. */
        glfwSwapBuffers(window);

    }
    glfwTerminate();
}
