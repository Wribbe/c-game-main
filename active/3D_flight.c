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

double mouse_x = 0;
double mouse_y = 0;
double mouse_x_prev = (double)WINDOW_WIDTH / 2.0f;
double mouse_y_prev = (double)WINDOW_HEIGHT / 2.0f;
GLfloat mouse_sensitivity = 0.005f;
GLfloat mouse_pitch = 0.0f;
GLfloat mouse_yaw = 0.0f;
GLfloat mouse_pitch_max = (M_PI/2)-0.1f;
GLfloat mouse_pitch_min = -(M_PI/2)+0.1f;

typedef GLfloat m4[4][4];

struct draw_object {
    GLsizei num_vertices;
    GLsizei num_uv_coords;
    GLfloat * points;
    GLfloat * uv_coords;
};

struct draw_object draw_object = {0};

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
    {0.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 1.0f},
};

struct v3 v3_camera_position = {{{0.0f, 0.0f, 3.0f}}};
struct v3 v3_camera_direction = {{{0.0f, 0.0f, -1.0f}}};
struct v3 v3_camera_up = {{{0.0f, 1.0f, 0.0f}}};

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

void
draw_object_set_vertice_data(struct draw_object * obj, struct v3 * points,
        size_t num_points)
{
    obj->num_vertices = num_points*3;
    size_t size_data = sizeof(GLfloat)*obj->num_vertices;
    if (obj->points == NULL) {
        obj->points = malloc(size_data);
    } else {
        obj->points = realloc(obj->points, size_data);
    }
    memcpy(obj->points, points, size_data);
}

void
draw_object_set_uv_coords(struct draw_object * obj, GLfloat * coords,
        size_t num_coords)
{
    size_t size_data = sizeof(GLfloat)*num_coords;
    obj->num_uv_coords = num_coords;
    if (obj->uv_coords == NULL) {
        obj->uv_coords = malloc(size_data);
    } else {
        obj->uv_coords = realloc(obj->uv_coords, size_data);
    }
    memcpy(obj->uv_coords, coords, size_data);
}

void
create_cube(struct draw_object * obj, struct v3 * center, GLfloat side)
{
    GLfloat hs = side/2.0f;
    GLfloat cx = center->x;
    GLfloat cy = center->y;
    GLfloat cz = center->z;

    struct v3 front_left_top = {{{cx-hs, cy+hs, cz+hs}}};
    struct v3 front_left_bot = {{{cx-hs, cy-hs, cz+hs}}};
    struct v3 front_right_top = {{{cx+hs, cy+hs, cz+hs}}};
    struct v3 front_right_bot = {{{cx+hs, cy-hs, cz+hs}}};

    struct v3 back_left_top = {{{cx-hs, cy+hs, cz-hs}}};
    struct v3 back_left_bot = {{{cx-hs, cy-hs, cz-hs}}};
    struct v3 back_right_top = {{{cx+hs, cy+hs, cz-hs}}};
    struct v3 back_right_bot = {{{cx+hs, cy-hs, cz-hs}}};

    struct v3 points[] = {
        // Front face.
        front_left_top,
        front_left_bot,
        front_right_top,
        front_right_top,
        front_left_bot,
        front_right_bot,
        // Back face.
        back_left_top,
        back_right_top,
        back_left_bot,
        back_left_bot,
        back_right_top,
        back_right_bot,
        // Left-side face.
        front_left_top,
        back_left_bot,
        front_left_bot,
        front_left_top,
        back_left_top,
        back_left_bot,
        // Right-side face.
        back_right_bot,
        front_right_top,
        front_right_bot,
        back_right_top,
        front_right_top,
        back_right_bot,
        // Top face.
        front_right_top,
        back_right_top,
        front_left_top,
        front_left_top,
        back_right_top,
        back_left_top,
        // bottom face.
        front_right_bot,
        front_left_bot,
        back_right_bot,
        back_right_bot,
        front_left_bot,
        back_left_bot,
    };

    GLfloat uv_coords[] = {
        // Front face.
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        // Front face.
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        // Front face.
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        // Front face.
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        // Front face.
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        // Front face.
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
    };

    draw_object_set_vertice_data(obj, points, SIZE(points));
    draw_object_set_uv_coords(obj, uv_coords, SIZE(uv_coords));
}

void
draw_objects()
{
    glDrawArrays(GL_TRIANGLES, 0, draw_object.num_vertices/3);
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
    size_t num_floats = draw_object.num_vertices;
    glBufferData(GL_ARRAY_BUFFER, num_floats*sizeof(GLfloat),
            draw_object.points, GL_STATIC_DRAW);

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

    /* Create secondary buffer for uv-coordinates. */
    GLuint vbo_uv = 0; // Vertex buffer object for uv-coordinates.
    glGenBuffers(1, &vbo_uv);
    /* Bind new buffer. */
    glBindBuffer(GL_ARRAY_BUFFER, vbo_uv);

    /* Populate new buffer with data. */
    glBufferData(GL_ARRAY_BUFFER, draw_object.num_uv_coords*sizeof(GLfloat),
            draw_object.uv_coords, GL_STATIC_DRAW);

    /* Setup and enable attribute pointer for uv-coordinates. */
    GLuint attribute_uv_coordinates = 1;
    glEnableVertexAttribArray(attribute_uv_coordinates);
    glVertexAttribPointer(
            attribute_uv_coordinates,
            2,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void*)0
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
"layout (location=1) in vec2 in_coords_uv;\n"
"\n"
"out vec2 coords_uv;\n"
"\n"
"void main() {\n"
"\n"
"  coords_uv = in_coords_uv;\n"
"  gl_Position = m4_mvp * vec4(vertex_data, 1.0f);\n"
"\n"
"}\n";

const GLchar * source_shader_fragment =
"#version 330 core\n"
"\n"
"uniform sampler2D sampler_texture;\n"
"\n"
"in vec2 coords_uv;\n"
"\n"
"out vec4 frag_color;\n"
"\n"
"void main() {\n"
"  frag_color = texture(sampler_texture, coords_uv);\n"
"  //frag_color = vec4(coords_uv, 0.0f, 1.0f);\n"
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
m4_equals(m4 m1, m4 m2) {
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
test_m4_equals(void)
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
            m4_equals(equal_1, equal_2) == GL_TRUE);

    mu_assert("Non-equal matrices returned as equal.",
            m4_equals(equal_1, non_equal) == GL_FALSE);
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
    mu_assert("Copied matrices are not equal.", m4_equals(to, from));
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
            m4_equals(result, a));

    m4_mul(result, identity, a);
    mu_assert("Identity matrix multiplied with matrix differs.",
            m4_equals(result, a));

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
            m4_equals(result, ab_correct));

    m4 ba_correct = {
        {40.0f, 40.0f, 40.0f, 40.0f},
        {30.0f, 30.0f, 30.0f, 30.0f},
        {20.0f, 20.0f, 20.0f, 20.0f},
        {10.0f, 10.0f, 10.0f, 10.0f},
    };

    m4_mul(result, b, a);
    mu_assert("Result of bxa was not correct.",
            m4_equals(result, ba_correct));

    return NULL;
}

static inline GLboolean
v3_equals(struct v3 * a, struct v3 * b)
{
    for (size_t i=0; i<3; i++) {
        if (a->raw[i] != b->raw[i]) {
            return GL_FALSE;
        }
    }
    return GL_TRUE;
}

const char *
test_v3_equals(void)
{
    struct v3 a = {{{1.0f, 2.0f, 3.0f}}};
    struct v3 b = {{{1.0f, 2.0f, 3.0f}}};
    struct v3 c = {{{1.0f, 2.0f, 1.0f}}};

    mu_assert("v3 a == v3 b but equals returned false.",
            v3_equals(&a,&b) == GL_TRUE);

    mu_assert("v3 a != v3 c but equals returned true.",
            v3_equals(&a,&c) == GL_FALSE);

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
            v3_equals(&correct, &result) == GL_TRUE);

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
            v3_equals(&correct, &result) == GL_TRUE);

    return NULL;
}

static inline void
v3_mulf(struct v3 * result, struct v3 * a, GLfloat f)
{
    for (size_t i=0; i<3; i++) {
        result->raw[i] = a->raw[i] * f;
    }
}

const char *
test_v3_mulf(void)
{
    struct v3 a = {{{1.0f, 2.0f, 3.0f}}};
    float f = 10.0f;

    struct v3 correct = {{{10.0f, 20.0f, 30.0f}}};
    struct v3 result = {0};

    v3_mulf(&result, &a, f);
    mu_assert("Result of v3 a * f not correct.",
            v3_equals(&correct, &result) == GL_TRUE);

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
            v3_equals(&correct, &result) == GL_TRUE);

    return NULL;
}

static inline GLfloat
v3_length(struct v3 * v)
{
    return sqrtf(v->x*v->x + v->y*v->y + v->z*v->z);
}

static inline void
m4_transpose(m4 result, m4 m)
{
    m4 temp = {0};
    for (size_t i=0; i<4; i++) {
        for (size_t j=0; j<4; j++) {
            temp[j][i] = m[i][j];
        }
    }
    m4_copy(result, temp);
}

const char *
test_m4_transpose(void)
{
    m4 m = {
        {1.0f, 5.0f, 6.0f, 7.0f},
        {2.0f, 1.0f, 0.0f, 0.0f},
        {3.0f, 0.0f, 1.0f, 9.0f},
        {4.0f, 0.0f, 8.0f, 1.0f},
    };

    m4 correct = {
        {1.0f, 2.0f, 3.0f, 4.0f},
        {5.0f, 1.0f, 0.0f, 0.0f},
        {6.0f, 0.0f, 1.0f, 8.0f},
        {7.0f, 0.0f, 9.0f, 1.0f},
    };

    m4 result = {0};

    m4_transpose(result, m);
    mu_assert("Transposition of m does not match correct matrix.",
            m4_equals(result, correct));

    return NULL;
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
        {0.0f,    0.0f,    -1.0,                0.0f},
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
            v3_equals(&result, &correct) == GL_TRUE);

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

static inline void
v3_cross(struct v3 * result, struct v3 * a, struct v3 * b)
{
    result->x = a->y*b->z - a->z*b->y;
    result->y = a->z*b->x - a->x*b->z;
    result->z = a->x*b->y - a->y*b->x;
}

const char *
test_v3_cross(void)
{
    struct v3 axis_x = {{{1.0f, 0.0f, 0.0f}}};
    struct v3 axis_y = {{{0.0f, 1.0f, 0.0f}}};
    struct v3 axis_z = {{{0.0f, 0.0f, 1.0f}}};

    struct v3 result = {0};

    v3_cross(&result, &axis_x, &axis_y);
    mu_assert("Cross product of x-axis and y-axis did not result in z-axis.",
            v3_equals(&result, &axis_z) == GL_TRUE);

    return NULL;
}

static inline void
m4_look_at(m4 result,
        struct v3 * camera_position,
        struct v3 * camera_looks_at,
        struct v3 * camera_up)
{
    struct v3 camera_right = {0};
    struct v3 camera_direction = {0};

    /* Calculate camera direction vector.
     *
     * This vector points towards the camera from the look-at-point, which
     * means it points in the opposite direction that the camera is facing.
     * This is important to consider when calculating the camera_right vector.
     */
    v3_subv3(&camera_direction, camera_position, camera_looks_at);

    /* Normalize before doing cross multiplication -> get unit vector back.*/
    v3_normalize(&camera_direction, &camera_direction);

    /* Use up and direction vector to calculate right vector.
     *
     * Using the right-hand-rule together with the knowledge that the
     * camera_direction will point towards the camera; doing up x direction
     * will result in a correct right vector without negation.
     */
    v3_cross(&camera_right, camera_up, &camera_direction);


    /* Construct the two parts of the view matrix. */
    m4 look_at_p1 = {
        {camera_right.x,     camera_right.y,     camera_right.z,     0.0f},
        {camera_up->x,       camera_up->y,       camera_up->z,       0.0f},
        {camera_direction.x, camera_direction.y, camera_direction.z, 0.0f},
        {0.0f,               0.0f,               0.0f,               1.0f},
    };

    m4 look_at_p2 = {
        {1.0f, 0.0f, 0.0f, -camera_position->x},
        {0.0f, 1.0f, 0.0f, -camera_position->y},
        {0.0f, 0.0f, 1.0f, -camera_position->z},
        {0.0f, 0.0f, 0.0f, 1.0f},
    };

    /* Calculate the final look-at matrix. */
    m4_mul(result, look_at_p1, look_at_p2);
}

GLuint
generate_texture_checkers(void)
{

    /* Checkers-pattern data. */
    GLubyte data_texture_checkerboard[] = {
        0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
        0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
        0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
        0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
        0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
        0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
        0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
        0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
        0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
        0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
    };

    /* Get unused texture id. */
    GLuint id_texture = 0;
    glGenTextures(1, &id_texture);

    /* Bind texture. */
    glBindTexture(GL_TEXTURE_2D, id_texture);

    /* Allocate storage for texture data. */
    glTexStorage2D(GL_TEXTURE_2D,// Target.
                   4,            // Number of texture mipmap levels.
                   GL_R8,        // Internal storage format for texture data.
                   8,            // Texture texel width.
                   8             // Texture texel height.
    );

    /* Specify the texture data. */
    glTexSubImage2D(GL_TEXTURE_2D,            // Target.
                    0,                        // First mipmap level.
                    0, 0,                     // X and Y offsets.
                    8, 8,                     // Texel width and height.
                    GL_RED,                   // Data format.
                    GL_UNSIGNED_BYTE,         // Data type.
                    data_texture_checkerboard // Pointer to data.
    );

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    return id_texture;
}

const char *
all_tests(void)
{
    mu_run_test(test_m4_equals);
    mu_run_test(test_m4_copy);
    mu_run_test(test_m4_mul);
    mu_run_test(test_v3_equals);
    mu_run_test(test_v3_addv3);
    mu_run_test(test_v3_mulv3);
    mu_run_test(test_v3_subv3);
    mu_run_test(test_v3_divf);
    mu_run_test(test_v3_normalize);
    mu_run_test(test_v3_cross);
    mu_run_test(test_m4_transpose);
    mu_run_test(test_v3_mulf);
    return NULL;
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

GLuint mod_index = 2;
GLboolean input_mode_fps = GL_TRUE;

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
                    if (key_down[GLFW_KEY_RIGHT_SHIFT]) {
                        input_mode_fps = !input_mode_fps;
                        printf("Input fps mode: %s\n", input_mode_fps ? "ON" : "OFF");
                    } else {
                        mod_index = (mod_index + 1) % 3;
                        printf("New mod axis: %d\n", mod_index);
                    }
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
        draw_object.points[mod_index] += mod_value;
        printf("new 1st %s-value: %f\n", mod_axis, draw_object.points[mod_index]);
    }

    GLfloat speed_camera = 0.05f;

    struct v3 v3_forward_back = {0};
    struct v3 v3_left_right = {0};

    // Calculate forward/backward vector.
    v3_mulf(&v3_forward_back, &v3_camera_direction, speed_camera);
    // Calculate left/right vector.
    v3_cross(&v3_left_right, &v3_camera_direction, &v3_camera_up);
    v3_normalize(&v3_left_right, &v3_left_right);
    v3_mulf(&v3_left_right, &v3_left_right, speed_camera);

    if (key_down[GLFW_KEY_W]) {
        if (!input_mode_fps) {
            m4_model[mod_index][3] += mod_value;
            printf("new model %s-value: %f\n", mod_axis, m4_model[mod_index][3]);
        } else {
            v3_addv3(&v3_camera_position, &v3_camera_position, &v3_forward_back);
        }
    }

    if (key_down[GLFW_KEY_A]) {
        if (!input_mode_fps) {
            draw_object.points[mod_index] -= mod_value;
            printf("new 1st %s-value: %f\n", mod_axis, draw_object.points[mod_index]);
        } else {
            v3_subv3(&v3_camera_position, &v3_camera_position, &v3_left_right);
        }
    }

    if (key_down[GLFW_KEY_S]) {
        if (!input_mode_fps) {
            m4_model[mod_index][3] -= mod_value;
            printf("new model %s-value: %f\n", mod_axis, m4_model[mod_index][3]);
        } else {
            v3_subv3(&v3_camera_position, &v3_camera_position, &v3_forward_back);
        }
    }

    if (key_down[GLFW_KEY_D]) {
        if (!input_mode_fps) {
            view_far -= mod_value;
            printf("new view_far value: %f\n", view_far);
        } else {
            v3_addv3(&v3_camera_position, &v3_camera_position, &v3_left_right);
        }
    }

    if (key_down[GLFW_KEY_E]) {
        view_far += mod_value;
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

GLboolean first_input_mouse = GL_TRUE;

static void
mouse_position_callback(GLFWwindow * window, double x, double y)
{
    UNUSED(window);
    mouse_x = x;
    mouse_y = y;

    if (first_input_mouse) {
        mouse_x_prev = x;
        mouse_y_prev = y;
        first_input_mouse = GL_FALSE;
    }

    double offset_x = mouse_x - mouse_x_prev;
    double offset_y = mouse_y_prev - mouse_y;

    mouse_x_prev = mouse_x;
    mouse_y_prev = mouse_y;

    offset_x *= mouse_sensitivity;
    offset_y *= mouse_sensitivity;

    mouse_yaw += offset_x;
    mouse_pitch += offset_y;

    if (mouse_pitch > mouse_pitch_max) {
        mouse_pitch = mouse_pitch_max;
    } else if (mouse_pitch < mouse_pitch_min) {
        mouse_pitch = mouse_pitch_min;
    }

    GLfloat cos_pitch = cosf(mouse_pitch);
    GLfloat cos_yaw = cosf(mouse_yaw);

    GLfloat sin_pitch = sinf(mouse_pitch);
    GLfloat sin_yaw = sinf(mouse_yaw);

    v3_camera_direction.x = cos_pitch * cos_yaw;
    v3_camera_direction.y = sin_pitch;
    v3_camera_direction.z = cos_pitch * sin_yaw;
    v3_normalize(&v3_camera_direction, &v3_camera_direction);
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
    glfwSetCursorPosCallback(window, mouse_position_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    current_window = window;

    return window;
}

void
m4_printf(m4 m)
{
    printf("\n");
    for (size_t i=0; i<4; i++) {
        for (size_t j=0; j<4; j++) {
            printf("%f,", m[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}


int
main(void)
{
    GLFWwindow * window = setup_glfw();

    glEnable(GL_DEPTH_TEST);

    /* Setup geometry. */
    struct v3 cube_center = {{{0.0f, 0.0f, 0.0f}}};
    GLfloat cube_side = 1.0f;
    create_cube(&draw_object, &cube_center, cube_side);

    /* Setup buffers and shaders. */
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

    /* Setup texture. */
    GLuint id_texture = generate_texture_checkers();
    glBindTexture(GL_TEXTURE_2D, id_texture);

    while (!glfwWindowShouldClose(window)) {

        /* Clear color and depth buffers. */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* Poll events. */
        glfwPollEvents();
        process_on_keyupdate_events();
        process_on_frame_events();

        /* Re-calculate matrices. */
        //m4_perspective(
        mat4x4_perspective(
                m4_projection,      // Where to store the projection matrix.
                view_yfov,          // Vertical field of view in radians.
                view_aspect_ratio,  // The aspect-ratio of the screen.
                view_near,          // The positional value of the near plane.
                view_far            // The positional value of the far plane.
        );
//        m4_transpose(m4_projection, m4_projection);

        struct v3 v3_camera_looks_at = {0};
        v3_addv3(&v3_camera_looks_at, &v3_camera_position, &v3_camera_direction);

        //m4_look_at(
        mat4x4_look_at(
                m4_view,            // Where to store result of computation.
                v3_camera_position.raw,// vec3 representing camera position.
                v3_camera_looks_at.raw,// vec3 representing where camera is looking.
                v3_camera_up.raw       // vec3 representing camera up direction.
        );
//        m4_transpose(m4_view, m4_view);

        mat4x4_mul(m4_mvp, m4_view, m4_model);
        mat4x4_mul(m4_mvp, m4_projection, m4_mvp);

        /* Load mvp matrix into vertex shader. */
        glUniformMatrix4fv(
                location_m4_mvp,    // Uniform location.
                1,                  // Number of matrices.
//                GL_TRUE,            // Supplied matrices are in row-major order?
                GL_FALSE,            // Supplied matrices are in row-major order?
                m4_mvp[0]           // Pointer to matrix data.
        );

        m4_transpose(m4_mvp, m4_mvp);
        m4_printf(m4_mvp);

        /* Update buffer data. */
        glBindBuffer(GL_ARRAY_BUFFER, 1);
        glBufferData(GL_ARRAY_BUFFER, draw_object.num_vertices*sizeof(GLfloat),
                draw_object.points, GL_STATIC_DRAW);

        /* Draw */
        draw_objects();

        /* Swap. */
        glfwSwapBuffers(window);

    }
    glfwTerminate();
}
