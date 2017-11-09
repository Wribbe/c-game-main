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

GLfloat view_yfov = M_PI/2;
GLfloat view_near = 0.1f;
GLfloat view_far = 100.0f;
GLfloat view_aspect_ratio = (float)WINDOW_WIDTH/(float)WINDOW_HEIGHT;

GLboolean key_down[500] = {0};

GLFWwindow * current_window = NULL;
GLboolean updated_keys = GL_FALSE;

typedef GLfloat m4[4][4];

struct v3 {
    union {
        struct {
            GLfloat x;
            GLfloat y;
            GLfloat z;
        };
        GLfloat raw[3];
    };
};


GLfloat vertices_triangle[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
};

m4 m4_projection = {
    {1.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 1.0f},
};

m4 m4_model = {
    {1.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 1.0f},
};

m4 m4_view = {
    {1.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, -3.0f},
    {0.0f, 0.0f, 0.0f, 1.0f},
};

struct v3 v3_camera_position = {{{0.0f, 0.0f, 3.0f}}};
struct v3 v3_camera_looks_at = {{{0.0f, 0.0f, 0.0f}}};
struct v3 v3_camera_vector_up = {{{0.0f, 1.0f, 0.0f}}};

#define SIZE_KEY_QUEUE 200
size_t last_key_queue = 0;
GLint a_key_queue[SIZE_KEY_QUEUE];

void
key_queue_append(int key)
{
    if (last_key_queue >= SIZE_KEY_QUEUE) {
        fprintf(stderr, "Too many keys in queue, not adding.\n");
        return;
    }
    a_key_queue[last_key_queue++] = key;
}

void
key_queue_reset(void)
{
    last_key_queue = 0;
}

static void
keyboard_key_callback(GLFWwindow * window, int key, int scancode, int action,
        int mods)
{
    UNUSED(mods);
    UNUSED(scancode);
    UNUSED(window);

    /* Ignore repeat actions. */
    if (action == GLFW_REPEAT) {
        return;
    }

    if (action == GLFW_PRESS) {
        key_down[key] = GL_TRUE;
    } else {
        key_down[key] = GL_FALSE;
    }
    key_queue_append(key);
}

GLuint mod_index = 2;

void
process_on_keyupdate_events(void)
{
    if (last_key_queue == 0) {
        return;
    }

    GLint * p_key = &a_key_queue[0];
    GLint * p_end = &a_key_queue[last_key_queue];
    for (; p_key < p_end; p_key++) {
        switch (*p_key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(current_window, GLFW_TRUE);
            break;
            case GLFW_KEY_TAB:
                if (key_down[GLFW_KEY_TAB]) {
                    mod_index = (mod_index + 1) % 3;
                    printf("New mod axis: %d\n", mod_index);
                }
            break;
        }
    }
    key_queue_reset();
}

void
process_on_frame_events(void)
{

    const GLchar * mod_axis = "";

    switch (mod_index) {
        case 0:
            mod_axis = "x";
        break;
        case 1:
            mod_axis = "y";
        break;
        case 2:
            mod_axis = "z";
        break;
    }

    GLfloat mod_value = 0.001f;
    if (key_down[GLFW_KEY_RIGHT_SHIFT]) {
        mod_value *= 100;
    }

    if (key_down[GLFW_KEY_Q]) {
        vertices_triangle[mod_index] += mod_value;
        printf("new 1st %s-value: %f\n", mod_axis, vertices_triangle[mod_index]);
    }
    if (key_down[GLFW_KEY_A]) {
        vertices_triangle[mod_index] -= mod_value;
        printf("new 1st %s-value: %f\n", mod_axis, vertices_triangle[mod_index]);
    }

    if (key_down[GLFW_KEY_W]) {
        m4_model[mod_index][3] += mod_value;
        printf("new model %s-value: %f\n", mod_axis, m4_model[mod_index][3]);
    }
    if (key_down[GLFW_KEY_S]) {
        m4_model[mod_index][3] -= mod_value;
        printf("new model %s-value: %f\n", mod_axis, m4_model[mod_index][3]);
    }

    if (key_down[GLFW_KEY_E]) {
        view_far += mod_value;
        printf("new view_far value: %f\n", view_far);
    }
    if (key_down[GLFW_KEY_D]) {
        view_far -= mod_value;
        printf("new view_far value: %f\n", view_far);
    }

    if (key_down[GLFW_KEY_R]) {
        v3_camera_position.raw[mod_index] -= mod_value;
        printf("new camera-%s-position: %f\n", mod_axis, v3_camera_position.raw[mod_index]);
    }
    if (key_down[GLFW_KEY_F]) {
        v3_camera_position.raw[mod_index] += mod_value;
        printf("new camera-%s-position: %f\n", mod_axis, v3_camera_position.raw[mod_index]);
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
"uniform mat4 m4_mvp;\n"
"\n"
"layout (location=0) in vec3 vertex_data;\n"
"\n"
"void main() {\n"
"\n"
"  gl_Position = m4_mvp * vec4(vertex_data, 1.0f);\n"
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

GLuint tests_run = 0;

#define mu_assert(message, test) do { if (!(test)) return message; } while (0)
#define mu_run_test(test) do { const char * message = test(); tests_run++; \
                                if (message) return message; } while (0)

static inline GLboolean
m4_compare(m4 m1, m4 m2) {
    for (size_t i=0; i<4; i++) {
        for (size_t j=0; j<4; j++) {
            if (m1[i][j] !=  m2[i][j]) {
                return GL_FALSE;
            }
        }
    }
    return GL_TRUE;
}

const char *
test_m4_compare(void)
{
    m4 equal_1 = {
        {1.0, 2.0, 3.0, 4.0},
        {1.0, 2.0, 3.0, 4.0},
        {1.0, 2.0, 3.0, 4.0},
        {1.0, 2.0, 3.0, 4.0},
    };

    m4 equal_2 = {
        {1.0, 2.0, 3.0, 4.0},
        {1.0, 2.0, 3.0, 4.0},
        {1.0, 2.0, 3.0, 4.0},
        {1.0, 2.0, 3.0, 4.0},
    };

    m4 non_equal = {
        {1.0, 2.0, 3.0, 4.0},
        {2.0, 2.0, 3.0, 4.0},
        {1.0, 1.0, 3.0, 4.0},
        {1.0, 2.0, 4.0, 9.0},
    };

    mu_assert("Equal matrices returned as non-equal.",
            m4_compare(equal_1, equal_2) == GL_TRUE);

    mu_assert("Non-equal matrices returned as equal.",
            m4_compare(equal_1, non_equal) == GL_FALSE);
    return NULL;
}

static inline void
m4_copy(m4 to, m4 from)
{
    for (size_t i=0; i<4; i++) {
        for (size_t j=0; j<4; j++) {
            to[i][j] = from[i][j];
        }
    }
}

const char *
test_m4_copy(void) {
    m4 to = {0};
    m4 from = {
        {1.0, 2.0, 3.0, 4.0},
        {2.0, 2.0, 3.0, 4.0},
        {1.0, 1.0, 3.0, 4.0},
        {1.0, 2.0, 4.0, 9.0},
    };
    m4_copy(to, from);
    mu_assert("Copied matrices are not equal.", m4_compare(to, from));
    return NULL;
}

static inline void
m4_mul(m4 result, m4 a, m4 b)
    /* Multiplication of row-major matrices. */
{
    m4 temp = {0};
    for (size_t i=0; i<4; i++) { // Iterates over rows.
        for (size_t j=0; j<4; j++) { // Iterates over columns.
            for (size_t k=0; k<4; k++) { // Iterates columns in b.
                temp[i][j] += a[i][k] * b[k][j];
            }
        }
    }
    m4_copy(result, temp);
}

const char *
test_m4_mul(void)
{
    m4 identity = {
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    };

    m4 a = {
        {1.0f, 1.0f, 1.0f, 1.0f},
        {2.0f, 2.0f, 2.0f, 2.0f},
        {3.0f, 3.0f, 3.0f, 3.0f},
        {4.0f, 4.0f, 4.0f, 4.0f},
    };

    m4 result = {0};

    m4_mul(result, a, identity);
    mu_assert("Matrix multiplied with identity matrix differs.",
            m4_compare(result, a));

    m4_mul(result, identity, a);
    mu_assert("Identity matrix multiplied with matrix differs.",
            m4_compare(result, a));

    m4 b = {
        {4.0f, 4.0f, 4.0f, 4.0f},
        {3.0f, 3.0f, 3.0f, 3.0f},
        {2.0f, 2.0f, 2.0f, 2.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
    };

    m4 ab_correct = {
        {10.0f, 10.0f, 10.0f, 10.0f},
        {20.0f, 20.0f, 20.0f, 20.0f},
        {30.0f, 30.0f, 30.0f, 30.0f},
        {40.0f, 40.0f, 40.0f, 40.0f},
    };

    m4_mul(result, a, b);
    mu_assert("Result of axb was not correct.",
            m4_compare(result, ab_correct));

    m4 ba_correct = {
        {40.0f, 40.0f, 40.0f, 40.0f},
        {30.0f, 30.0f, 30.0f, 30.0f},
        {20.0f, 20.0f, 20.0f, 20.0f},
        {10.0f, 10.0f, 10.0f, 10.0f},
    };

    m4_mul(result, b, a);
    mu_assert("Result of bxa was not correct.",
            m4_compare(result, ba_correct));

    return NULL;
}

static inline GLboolean
v3_compare(struct v3 a, struct v3 b)
{
    for (size_t i=0; i<3; i++) {
        if (a.raw[i] != b.raw[i]) {
            return GL_FALSE;
        }
    }
    return GL_TRUE;
}

const char *
test_v3_compare(void)
{
    struct v3 a = {{{1.0f, 2.0f, 3.0f}}};
    struct v3 b = {{{1.0f, 2.0f, 3.0f}}};
    struct v3 c = {{{1.0f, 2.0f, 1.0f}}};

    mu_assert("v3 a == v3 b but compare returned false.",
            v3_compare(a,b) == GL_TRUE);

    mu_assert("v3 a != v3 c but compare returned true.",
            v3_compare(a,c) == GL_FALSE);

    return NULL;
}

static inline void
v3_addv3(struct v3 * result, struct v3 * a, struct v3 * b)
{
    for (size_t i=0; i<3; i++) {
        result->raw[i] = a->raw[i] + b->raw[i];
    }
}

const char *
test_v3_addv3(void)
{
    struct v3 a = {{{1.0f, 2.0f, 3.0f}}};
    struct v3 b = {{{1.0f, 2.0f, 3.0f}}};

    struct v3 correct = {{{2.0f, 4.0f, 6.0f}}};
    struct v3 result = {0};

    v3_addv3(&result, &a, &b);
    mu_assert("Result of v3 a + v3 b not correct.",
            v3_compare(correct, result) == GL_TRUE);

    return NULL;
}

static inline void
v3_mulv3(struct v3 * result, struct v3 * a, struct v3 * b)
{
    for (size_t i=0; i<3; i++) {
        result->raw[i] = a->raw[i] * b->raw[i];
    }
}

const char *
test_v3_mulv3(void)
{
    struct v3 a = {{{1.0f, 2.0f, 3.0f}}};
    struct v3 b = {{{1.0f, 2.0f, 3.0f}}};

    struct v3 correct = {{{1.0f, 4.0f, 9.0f}}};
    struct v3 result = {0};

    v3_mulv3(&result, &a, &b);
    mu_assert("Result of v3 a * v3 b not correct.",
            v3_compare(correct, result) == GL_TRUE);

    return NULL;
}

static inline void
v3_subv3(struct v3 * result, struct v3 * a, struct v3 * b)
{
    for (size_t i=0; i<3; i++) {
        result->raw[i] = a->raw[i] - b->raw[i];
    }
}

const char *
test_v3_subv3(void)
{
    struct v3 a = {{{1.0f, 2.0f, 3.0f}}};
    struct v3 b = {{{4.0f, 4.0f, 4.0f}}};

    struct v3 correct = {{{-3.0f, -2.0f, -1.0f}}};
    struct v3 result = {0};

    v3_subv3(&result, &a, &b);
    mu_assert("Result of v3 a - v3 b not correct.",
            v3_compare(correct, result) == GL_TRUE);

    return NULL;
}

static inline GLfloat
v3_length(struct v3 * v)
{
    return sqrtf(v->x*v->x + v->y*v->y + v->z*v->z);
}

static inline void
m4_perspective(m4 result,
        GLfloat vertical_fov,
        GLfloat aspect_ratio,
        GLfloat plane_near,
        GLfloat plane_far)
    /* Creates perspective matrix based on symmetric viewing volume. The
     * returned matrix is in row-major format.  */
{
    GLfloat near_top = tanf(vertical_fov/2)*plane_near;
    GLfloat near_right = near_top * aspect_ratio;

    GLfloat n_div_r = plane_near/near_right;
    GLfloat n_div_t = plane_near/near_top;

    GLfloat sum_fn = plane_far + plane_near;
    GLfloat diff_fn = plane_far - plane_near;
    GLfloat mul_fn = plane_far*plane_near;

    m4 perspective = {
        {n_div_r, 0.0f,    0.0f,                0.0f},
        {0.0f,    n_div_t, 0.0f,                0.0f},
        {0.0f,    0.0f,    -sum_fn/diff_fn,     -(2*mul_fn)/diff_fn},
        {0.0f,    0.0f,    -1.0, 0.0f},
    };

    m4_copy(result, perspective);
}

static inline void
v3_divf(struct v3 * result, struct v3 * v, GLfloat f)
{
    for (size_t i=0; i<3; i++) {
        result->raw[i] = v->raw[i] / f;
    }
}

const char *
test_v3_divf(void)
{
    struct v3 a = {{{4.0f, 6.0f, 8.0f}}};
    GLfloat divisor = 2.0f;
    struct v3 result = {0};
    struct v3 correct = {{{2.0f, 3.0f, 4.0f}}};

    v3_divf(&result, &a, divisor);
    mu_assert("Vector divided by float did not match correct.\n",
            v3_compare(result, correct) == GL_TRUE);

    return NULL;
}

static inline void
v3_normalize(struct v3 * result, struct v3 * v)
{
    GLfloat length = v3_length(v);
    v3_divf(result, v, length);
}

const char *
test_v3_normalize(void)
{
    struct v3 a = {{{1.0f, 2.0f, 2.0f}}}; // Length == 3.
    struct v3 result = {{{1.0f, 2.0f, 2.0f}}};

    GLfloat length_a = v3_length(&a);
    mu_assert("Length was not 3.", length_a == 3.0f);

    v3_normalize(&result, &a);
    mu_assert("Length was not 1 after normalization.",
            v3_length(&result) == 1.0f);

    return NULL;
}

const char *
all_tests(void)
{
    mu_run_test(test_m4_compare);
    mu_run_test(test_m4_copy);
    mu_run_test(test_m4_mul);
    mu_run_test(test_v3_compare);
    mu_run_test(test_v3_addv3);
    mu_run_test(test_v3_mulv3);
    mu_run_test(test_v3_subv3);
    mu_run_test(test_v3_divf);
    mu_run_test(test_v3_normalize);
    return NULL;
}

static inline void
m4_look_at(m4 result,
        struct v3 * camera_position,
        struct v3 * camera_looks_at,
        struct v3 * camera_vector_up)
{
}

int
tests_ok(void)
{
    const char * results = all_tests();
    if (results != 0) {
        printf("%s\n", results);
    } else {
        printf("All tests passed!\n");
    }
    printf("Tests run: %d\n", tests_run);
    return results == NULL;
}

int
main(void)
{
    GLFWwindow * window = setup_glfw();

    GLuint id_vao = setup_buffers();
    GLuint id_program = setup_shaders();

    glBindVertexArray(id_vao);
    glUseProgram(id_program);

    m4 m4_mvp = {
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    };

    GLuint location_m4_mvp = glGetUniformLocation(id_program, "m4_mvp");

    /* Run tests. */
    if (!tests_ok()) {
        fprintf(stderr, "There were errors running the test-suit, aborting.\n");
        exit(EXIT_FAILURE);
    }

    while (!glfwWindowShouldClose(window)) {

        /* Clear color and depth buffers. */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* Poll events. */
        glfwPollEvents();
        process_on_keyupdate_events();
        process_on_frame_events();

        /* Re-calculate matrices. */
        m4_perspective(
                m4_projection,      // Where to store the projection matrix.
                view_yfov,          // Vertical field of view in radians.
                view_aspect_ratio,  // The aspect-ratio of the screen.
                view_near,          // The positional value of the near plane.
                view_far            // The positional value of the far plane.
        );

        m4_look_at(
                m4_view,            // Where to store result of computation.
                &v3_camera_position,// vec3 representing camera position.
                &v3_camera_looks_at,// vec3 representing where camera is looking.
                &v3_camera_vector_up// vec3 representing camera up direction.
        );

        m4_mul(m4_mvp, m4_view, m4_model);
        m4_mul(m4_mvp, m4_projection, m4_mvp);

        /* Load mvp matrix into vertex shader. */
        glUniformMatrix4fv(
                location_m4_mvp,    // Uniform location.
                1,                  // Number of matrices.
                GL_TRUE,            // Supplied matrices are in row-major order?
                m4_mvp[0]           // Pointer to matrix data.
        );

        /* Update buffer data. */
        glBindBuffer(GL_ARRAY_BUFFER, 1);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_triangle),
                vertices_triangle, GL_STATIC_DRAW);

        /* Draw */
        draw_objects();

        /* Swap. */
        glfwSwapBuffers(window);

    }
}
