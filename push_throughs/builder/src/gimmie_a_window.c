#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "GL/gl3w.h"
#include "GLFW/glfw3.h"

#define WIDTH 800
#define HEIGHT 600

#define M_PI 3.14159265358979323846
#define TRANSPOSE GL_TRUE

#define SIZE(x) sizeof(x)/sizeof(x[0])
#define DATv(x) &x[0]
#define DATm(x) &(x.data[0][0])
#define UNUSED(x) (void)x

struct vertices {
    size_t size;
    size_t vertices;
    GLfloat * data;
};

const char * source_frag_simple =\
    "#version 330 core\n"
    "\n"
    "uniform vec4 uniform_color;\n"
    "\n"
    "out vec4 color;\n"
    "\n"
    "void main() {\n"
    "   color = uniform_color;\n"
    "}\n";

const char * source_vert_simple =\
    "#version 330 core\n"
    "\n"
    "layout (location = 0) in vec3 position;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "\n"
    "void main() {\n"
    "   gl_Position = projection * view * model * vec4(position, 1.0f);\n"
    "}\n";

GLfloat color_cube[] = {
    1.0f,
    0.0f,
    1.0f,
    1.0f,
};

GLfloat color_floor[] = {
    1.0f,
    0.0f,
    0.0f,
    1.0f,
};

char * strtok_r(
        char * string,
        const char * delimiter,
        char ** pointer_next)
{
    char * ret = NULL;

    if (string == NULL) {
        string = *pointer_next;
    }

    string += strspn(string, delimiter);

    if (*string == '\0') {
        return NULL;
    }

    ret = string;

    string += strcspn(string, delimiter);

    if (*string) {
        *string++ = '\0';
    }

    *pointer_next = string;

    return ret;
}

struct vertices vertices_cube  = {0};

const char data_cube[] =\
"-0.5f, -0.5f, -0.5f,\n"
" 0.5f, -0.5f, -0.5f,\n"
" 0.5f,  0.5f, -0.5f,\n"
" 0.5f,  0.5f, -0.5f,\n"
"-0.5f,  0.5f, -0.5f,\n"
"-0.5f, -0.5f, -0.5f,\n"
"\n"
"-0.5f, -0.5f,  0.5f,\n"
" 0.5f, -0.5f,  0.5f,\n"
" 0.5f,  0.5f,  0.5f,\n"
" 0.5f,  0.5f,  0.5f,\n"
"-0.5f,  0.5f,  0.5f,\n"
"-0.5f, -0.5f,  0.5f,\n"
"\n"
"-0.5f,  0.5f,  0.5f,\n"
"-0.5f,  0.5f, -0.5f,\n"
"-0.5f, -0.5f, -0.5f,\n"
"-0.5f, -0.5f, -0.5f,\n"
"-0.5f, -0.5f,  0.5f,\n"
"-0.5f,  0.5f,  0.5f,\n"
"\n"
" 0.5f,  0.5f,  0.5f,\n"
" 0.5f,  0.5f, -0.5f,\n"
" 0.5f, -0.5f, -0.5f,\n"
" 0.5f, -0.5f, -0.5f,\n"
" 0.5f, -0.5f,  0.5f,\n"
" 0.5f,  0.5f,  0.5f,\n"
"\n"
"-0.5f, -0.5f, -0.5f,\n"
" 0.5f, -0.5f, -0.5f,\n"
" 0.5f, -0.5f,  0.5f,\n"
" 0.5f, -0.5f,  0.5f,\n"
"-0.5f, -0.5f,  0.5f,\n"
"-0.5f, -0.5f, -0.5f,\n"
"\n"
"-0.5f,  0.5f, -0.5f,\n"
" 0.5f,  0.5f, -0.5f,\n"
" 0.5f,  0.5f,  0.5f,\n"
" 0.5f,  0.5f,  0.5f,\n"
"-0.5f,  0.5f,  0.5f,\n"
"-0.5f,  0.5f, -0.5f,\n";

GLfloat vertices_floor[] = {
    -4.0f, -1.0f, -4.0f,
     4.0f, -1.0f, -4.0f,
     4.0f, -1.0f,  4.0f,
     // Second part.
    -4.0f, -1.0f, -4.0f,
    -4.0f, -1.0f,  4.0f,
     4.0f, -1.0f,  4.0f,
};

struct v3 {
    GLfloat x;
    GLfloat y;
    GLfloat z;
};

struct v3 v3_sub(struct v3 v1, struct v3 v2)
{
    return (struct v3){v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
}

double v3_magnitude(struct v3 v)
{
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

struct v3 v3_normalize(struct v3 v)
{
    double magnitude = v3_magnitude(v);
    return (struct v3){v.x/magnitude, v.y/magnitude, v.z/magnitude};
}

struct v3 v3_cross(struct v3 v1, struct v3 v2)
{
    return (struct v3){
        v1.y*v2.z - v1.z*v2.y,
        v1.z*v2.x - v1.x*v2.z,
        v1.x*v2.y - v1.y*v2.x
    };
}

struct m4 {
    GLfloat data[4][4];
};

struct m4 m4_mul(struct m4 * A, struct m4 * B)
{
    struct m4 result = {0};
    for (size_t i=0; i<4; i++) {
        for (size_t j=0; j<4; j++) {
            for (size_t k=0; k<4; k++) {
                result.data[i][j] += A->data[i][k] * B->data[k][j];
            }
        }
    }
    return result;
}

struct m4 look_at(struct v3 * right,
           struct v3 * up,
           struct v3 * direction,
           struct v3 * position)
{
    struct m4 A = {{
        {right->x, right->y, right->z, 0.0f},
        {up->x, up->y, up->z, 0.0f},
        {direction->x, direction->y, direction->z, 0.0f},
        {0.0f, 0.0f, 0.0f, 0.0f}}
    };
    struct m4 B = {{
        {1.0f, 0.0f, 0.0f, -position->x},
        {0.0f, 1.0f, 0.0f, -position->y},
        {0.0f, 0.0f, 1.0f, -position->z},
        {0.0f, 0.0f, 0.0f, 1.0f}
    }};
    return m4_mul(&A,&B);
}

struct m4 m4_eye(void)
{
    return (struct m4) {{
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    }};
}

struct m4 m4_perspective(GLfloat near, GLfloat far, GLfloat rad_fov, GLfloat aspect_ratio)
{
    float tan_fov_half = tanf(rad_fov/2.0f);
    struct m4 perspective = m4_eye();

    perspective.data[0][0] = tan_fov_half / aspect_ratio;
    perspective.data[1][1] = tan_fov_half;
    perspective.data[2][2] = (far+near)/(near-far);
    perspective.data[2][3] = (2*far*near)/(near-far);
    perspective.data[3][2] = -1;
    perspective.data[3][3] = 0;

    return perspective;
}

void m4_translate(struct m4 * m, struct v3 v_translate)
{
    m->data[0][3] = v_translate.x;
    m->data[1][3] = v_translate.y;
    m->data[2][3] = v_translate.z;
}

struct m3 {
    float data[3][3];
};

void m3_smul(float scalar, struct m3 * m)
{
    for (size_t i=0; i<3; i++) {
        for (size_t j=0; j<3; j++) {
            m->data[i][j] *= scalar;
        }
    }
}

struct m3 m3_mul(struct m3 * A, struct m3 * B)
{
    struct m3 result = {0};
    for (size_t i=0; i<3; i++) {
        for (size_t j=0; j<3; j++) {
            for (size_t k=0; k<3; k++) {
                result.data[i][j] += A->data[i][k] * B->data[k][j];
            }
        }
    }
    return result;
}

struct m4 m3_to_m4(struct m3 * m)
{
    struct m4 converted = m4_eye();
    for (size_t i=0; i<3; i++) {
        for (size_t j=0; j<3; j++) {
            converted.data[i][j] = m->data[i][j];
        }
    }
    return converted;
}

void m4_write(struct m4 * dest, struct m4 * source)
{
    for (size_t i=0; i<4; i++) {
        for (size_t j=0; j<4; j++) {
            dest->data[i][j] = source->data[i][j];
        }
    }
}

struct m3 m3_eye(void)
{
    return (struct m3) {{
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
    }};
}

void m3_write(struct m3 * dest, struct m3 * source)
{
    for (size_t i=0; i<3; i++) {
        for (size_t j=0; j<3; j++) {
            dest->data[i][j] = source->data[i][j];
        }
    }
}

void m4_rotate(struct m4 * m, float rad_angle, struct v3 axis)
{
    float s = sinf(rad_angle);
    float c = cosf(rad_angle);

    struct v3 na = v3_normalize(axis);

    struct m4 rotation = {{
        {c + na.x*na.x*(1 - c),     na.x*na.y*(1 - c) - na.z*s, na.x*na.z*(1-c)+na.y*s,  0.0f},
        {na.y*na.x*(1-c)+na.z*s,    c+na.y*na.y*(1-c),          na.y*na.z*(1-c)-na.x*s,  0.0f},
        {na.z*na.x*(1-c)-na.y*s,    na.z*na.y*(1-c)+na.x*s,     c+na.z*na.z*(1-c),       0.0f},
        {0.0f,                      0.0f,                       0.0f,                    1.0f}
    }};

    struct m4 result = m4_mul(m, &rotation);
    m4_write(m, &result);

}

struct min_max {
    float min;
    float max;
};

struct pos_box {
    struct min_max x;
    struct min_max y;
    struct min_max z;
};

struct object {
    struct pos_box bound;
    struct vertices vertices;
};

struct pos_box pos_box_get(struct vertices * vertices)
{

    float * pointer = vertices->data;
    float * end = pointer + vertices->size;

    float x_min = *pointer;
    float x_max = *pointer++;

    float y_min = *pointer;
    float y_max = *pointer++;

    float z_min = *pointer;
    float z_max = *pointer++;

    while (pointer <= end) {

        float x = *pointer++;
        float y = *pointer++;
        float z = *pointer++;

        if (x < x_min) {
            x_min = x;
        }
        if (x > x_max) {
            x_max = x;
        }

        if (y < y_min) {
            y_min = y;
        }
        if (y > y_max) {
            y_max = y;
        }

        if (z < z_min) {
            z_min = z;
        }
        if (z > z_max) {
            z_max = z;
        }
    }

    return (struct pos_box){
        {x_min, x_max},
        {y_min, y_max},
        {z_min, z_max},
    };
}

void load_vertices(struct vertices * vertices, char * string)
{
    char * saveptr = NULL;
    const char * delimiter = ",";

    char * token = strtok_r(string, delimiter, &saveptr);

    size_t data_size = 512;
    size_t num_points = 0;

    vertices->data = malloc(data_size*sizeof(float));
    if (!vertices->data) {
        fprintf(stderr, "Could not allocate enough data for loading vertices.\n");
        exit(EXIT_FAILURE);
    }
    float * last_entry = vertices->data;

    while (token != NULL) {
        *last_entry = strtof(token, NULL);
        last_entry++;
        num_points++;
        if (num_points >= data_size) {
            data_size *= 2;
            vertices->data = realloc(vertices->data, data_size*sizeof(float));
            if (!vertices->data) {
                fprintf(stderr, "Not enough memory to reallocate data.\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok_r(NULL, delimiter, &saveptr);
    }
    if (num_points < data_size) {
        vertices->data = realloc(vertices->data, num_points*sizeof(float));
    }
    vertices->size = num_points;
    vertices->vertices = num_points/3;
}

#define NUM_KEYS 512
bool key_down[NUM_KEYS] = {0};

void callback_keys(GLFWwindow * window, int key, int scancode, int action, int mods)
{
    UNUSED(scancode);

    if (key >= NUM_KEYS) {
        fprintf(stderr, "Key %d is out of bounds for key array!\n", key);
        return;
    }

    if (action == GLFW_PRESS) {
        key_down[key] = true;
    } else if (action == GLFW_RELEASE) {
        key_down[key] = false;
    }

}

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

    window = glfwCreateWindow(WIDTH, HEIGHT, "HELLO WORLD", NULL, NULL);
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

    // Set key callback for window.
    glfwSetKeyCallback(window, callback_keys);

    // Load data into vertices structs.
    size_t size_data_cube = sizeof(data_cube);
    char * dynamic_data_cube = malloc(size_data_cube);
    memcpy(dynamic_data_cube, data_cube, size_data_cube);
    load_vertices(&vertices_cube, dynamic_data_cube);
    struct object obj_cube = {0};
    obj_cube.vertices = vertices_cube;
    obj_cube.bound = pos_box_get(&vertices_cube);

    // Set up cube.
    GLuint VBO_cube, VAO_cube;
    glGenBuffers(1, &VBO_cube);
    glGenVertexArrays(1, &VAO_cube);

    glBindVertexArray(VAO_cube);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_cube);
    glBufferData(GL_ARRAY_BUFFER, vertices_cube.size*sizeof(float), vertices_cube.data, GL_STATIC_DRAW);

    // Set up vertex attribute pointers.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // set up floor.
    GLuint VBO_floor, VAO_floor;
    glGenBuffers(1, &VBO_floor);
    glGenVertexArrays(1, &VAO_floor);

    glBindVertexArray(VAO_floor);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_floor);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_floor), vertices_floor, GL_STATIC_DRAW);

    // Set up vertex attribute pointers.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Unbind vertex array.
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

    // Set up camera directions.
    struct v3 camera_position = {0.0f, 0.0f, 3.0f};
    struct v3 camera_target = {0.0f, 0.0f, 0.0f};
    struct v3 camera_direction = v3_normalize(v3_sub(camera_position, camera_target));

    struct v3 camera_up = {0.0f, 1.0f, 0.0f};
    struct v3 camera_right = v3_normalize(v3_cross(camera_up, camera_direction));

    glEnable(GL_DEPTH_TEST);

    struct v3 coords_cube = {0};

    glfwSwapInterval(0);

    double time_prev = glfwGetTime();
    double time_current = 0;
    double time_delta = 0;

    struct m4 mat_projection = m4_perspective(0.1f, 100.0f, M_PI*0.5f, (double)WIDTH/(double)HEIGHT);

    while(!glfwWindowShouldClose(window)) {

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        GLint uniform_model = glGetUniformLocation(program_shader_simple, "model");
        GLint uniform_view = glGetUniformLocation(program_shader_simple, "view");
        GLint uniform_projection = glGetUniformLocation(program_shader_simple, "projection");
        GLint uniform_color = glGetUniformLocation(program_shader_simple, "uniform_color");

        struct m4 mat_view = m4_eye();
        m4_translate(&mat_view, (struct v3){0.0f, 0.0f, -3.0f});

        struct m4 mat_model = m4_eye();
//        m4_rotate(&mat_model, -M_PI*0.4*(GLfloat)glfwGetTime(), (struct v3){0.5f, 1.0f, 0.0f});
        m4_translate(&mat_model, coords_cube);

        if (key_down[GLFW_KEY_ESCAPE]) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        // Set up time delta.
        time_current = glfwGetTime();
        time_delta = time_current - time_prev;
        time_prev = time_current;

        float next_cube_y = coords_cube.y - 0.4f * time_delta;
        if (next_cube_y + obj_cube.bound.y.min >= -1.0) {
            coords_cube.y = next_cube_y;
        }

        glUniformMatrix4fv(uniform_model, 1, TRANSPOSE, DATm(mat_model));
        glUniformMatrix4fv(uniform_view, 1, TRANSPOSE, DATm(mat_view));
        glUniformMatrix4fv(uniform_projection, 1, TRANSPOSE, DATm(mat_projection));

        glBindVertexArray(VAO_cube);
        glUniform4fv(uniform_color, 1, DATv(color_cube));
        glDrawArrays(GL_TRIANGLES, 0, vertices_cube.vertices);

        glBindVertexArray(VAO_floor);

        struct m4 mat_model_neutral = m4_eye();
        glUniformMatrix4fv(uniform_model, 1, TRANSPOSE, DATm(mat_model_neutral));

        glUniform4fv(uniform_color, 1, DATv(color_floor));
        glDrawArrays(GL_TRIANGLES, 0, SIZE(vertices_floor));

        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    glfwTerminate();
    return 0;
    // check:  http://www.dyn4j.org/2010/01/sat/ for SAT
}
