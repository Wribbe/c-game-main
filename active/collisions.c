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

double time_delta = 0.0f;

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
"uniform mat4 m4_mvp;\n"
"\n"
"layout (location=0) in vec3 vertex_data;\n"
"\n"
"void main() {\n"
"  gl_Position = m4_mvp * vec4(vertex_data, 1.0f);\n"
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

mat4x4 m4_projection = {
    {1.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 1.0f},
};

mat4x4 m4_view = {
    {1.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 1.0f},
};

mat4x4 m4_model = {
    {1.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 1.0f},
};

mat4x4 m4_mvp = {
    {1.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 1.0f},
};


vec3 vec3_camera_up = {0.0f, 1.0f, 0.0f};
vec3 vec3_camera_direction  = {0.0f, 0.0f, -1.0f};
vec3 vec3_camera_position = {0.0f, 0.0f, 13.0f};

GLfloat speed_camera = 10.0f;

double mouse_x_prev = (float)WINDOW_WIDTH/2.0f;
double mouse_y_prev = (float)WINDOW_HEIGHT/2.0f;
GLboolean input_mouse_first = GL_TRUE;

double mouse_pitch = 0;
double mouse_yaw = 0;

double two_pi = 2*M_PI;


GLboolean key_down[512] = {0};

static void
callback_mouse_position_method(GLFWwindow * w, double x, double y)
{
    if (input_mouse_first) {
        glfwSetCursorPos(w, mouse_x_prev, mouse_y_prev);
        input_mouse_first = GL_FALSE;
        return;
    }

    double diff_x = x - mouse_x_prev;
    double diff_y = mouse_y_prev - y;

    double mouse_sensitivity = 0.0005f;

    diff_x *= mouse_sensitivity;
    diff_y *= mouse_sensitivity;

    mouse_pitch += diff_y;
    mouse_yaw += diff_x;

    mouse_x_prev = x;
    mouse_y_prev = y;

    /* diff_x = 0, and diff_y = 0 should result in {0,0,-1} direction. */
    vec3_camera_direction[0] = cos(mouse_pitch) * sin(mouse_yaw);
    vec3_camera_direction[1] = sin(mouse_pitch);
    vec3_camera_direction[2] = -cos(mouse_pitch) * cos(mouse_yaw);
}

static void
callback_key_method(GLFWwindow * window, int key, int scancode, int action,
        int mods)
{

    if (action == GLFW_REPEAT) {
        return;
    }

    UNUSED(mods);
    UNUSED(scancode);

    if (key_down[GLFW_KEY_ESCAPE]) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    } else {
        key_down[key] = action == GLFW_PRESS ? GL_TRUE : GL_FALSE;
    }
}

void
process_frame_events(void)
{
    GLfloat speed_camera_delta = speed_camera * time_delta;

    vec3 vec3_forward = {0};
    vec3_scale(vec3_forward, vec3_camera_direction, speed_camera_delta);

    vec3 vec3_right = {0};
    vec3_mul_cross(vec3_right, vec3_camera_direction, vec3_camera_up);
    vec3_norm(vec3_right, vec3_right);
    vec3_scale(vec3_right, vec3_right, speed_camera_delta);
    if (key_down[GLFW_KEY_W]) { // Go forward in camera direction.
        vec3_add(vec3_camera_position, vec3_camera_position,
                vec3_forward);
    }
    if (key_down[GLFW_KEY_S]) { // Go backward in camera direction.
        vec3_sub(vec3_camera_position, vec3_camera_position,
                vec3_forward);
    }
    if (key_down[GLFW_KEY_D]) { // Go along right direction.
        vec3_add(vec3_camera_position, vec3_camera_position,
                vec3_right);
    }
    if (key_down[GLFW_KEY_A]) { // Go along opposite to right direction.
        vec3_sub(vec3_camera_position, vec3_camera_position,
                vec3_right);
    }
}

struct object_drawable {
    GLsizei num_vertices;
    GLsizei num_indices;
    GLsizei num_normals;
    GLfloat * vertices;
    GLfloat * normals;
    GLuint * indices;
};

struct object {
    struct object_drawable * drawable;
    mat4x4 model;
    vec3 velocity;
};

#define MAX_DRAWABLE_OBJECTS 300
struct object_drawable r_objects_drawable[MAX_DRAWABLE_OBJECTS];
GLsizei object_drawable_first_empty = 0;

#define MAX_OBJECTS 200
struct object r_objects[MAX_OBJECTS];
GLsizei object_first_empty = 0;

int
main(void)
{
    GLFWwindow * window = setup_glfw();

    GLuint id_shader_program = setup_shaders();
    glUseProgram(id_shader_program);

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
    //const GLchar * filename = "cube.obj";
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

    glfwSetKeyCallback(window, callback_key_method);
    glfwSetCursorPosCallback(window, callback_mouse_position_method);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    GLuint ulocation_m4_mvp = glGetUniformLocation(id_shader_program,
            "m4_mvp");

    /* Set projection matrix as a perspective matrix. */
    mat4x4_perspective(m4_projection, M_PI/4,
            (float)WINDOW_WIDTH/(float)WINDOW_HEIGHT, 0.1f, 100.0f);

    double time_prev = glfwGetTime();
    double time_now = 0;

    while (!glfwWindowShouldClose(window)) {

        time_now = glfwGetTime();
        time_delta = time_now - time_prev;
        time_prev = time_now;

        /* Clear color and depth buffers. */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* Poll events. */
        glfwPollEvents();
        process_frame_events();


        /* Re-calculate view matrix. */
        vec3 vec3_camera_center = {0};
        vec3_add(vec3_camera_center, vec3_camera_position, vec3_camera_direction);
        mat4x4_look_at(m4_view, vec3_camera_position, vec3_camera_center,
                vec3_camera_up);

        /* Re-calculate mvp matrix. */
        mat4x4_mul(m4_mvp, m4_view, m4_model);
        mat4x4_mul(m4_mvp, m4_projection, m4_mvp);

        /* Supply mvp matrix to vertex shader. */
        glUniformMatrix4fv(
                ulocation_m4_mvp,
                1,
                GL_FALSE,
                m4_mvp[0]);

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
