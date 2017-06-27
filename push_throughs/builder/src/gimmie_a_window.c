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

#define GRAVITY 9.806f * 0.8f

double time_delta = 0;
double val_cube_speed = 4.2f;
double val_cube_max_speed = 4.2f;
double val_cube_friction = 16.0f;

struct vertices {
    size_t size;
    size_t points;
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

char data_cube[] =\
"-0.125f,   -0.5f, -0.025f,\n"
" 0.125f,   -0.5f, -0.025f,\n"
" 0.125f,    0.5f, -0.025f,\n"
"-0.125f,   -0.5f, -0.025f,\n"
" 0.125f,    0.5f, -0.025f,\n"
"-0.125f,    0.5f, -0.025f,\n"
"-0.125f,   -0.5f,  0.025f,\n"
" 0.125f,   -0.5f,  0.025f,\n"
" 0.125f,    0.5f,  0.025f,\n"
"-0.125f,   -0.5f,  0.025f,\n"
" 0.125f,    0.5f,  0.025f,\n"
"-0.125f,    0.5f,  0.025f,\n"
"-0.125f,   -0.5f, -0.025f,\n"
"-0.125f,    0.5f, -0.025f,\n"
"-0.125f,    0.5f,  0.025f,\n"
"-0.125f,   -0.5f, -0.025f,\n"
"-0.125f,    0.5f,  0.025f,\n"
"-0.125f,   -0.5f,  0.025f,\n"
" 0.125f,   -0.5f, -0.025f,\n"
" 0.125f,    0.5f, -0.025f,\n"
" 0.125f,    0.5f,  0.025f,\n"
" 0.125f,   -0.5f, -0.025f,\n"
" 0.125f,    0.5f,  0.025f,\n"
" 0.125f,   -0.5f,  0.025f,\n"
"-0.125f,   -0.5f, -0.025f,\n"
" 0.125f,   -0.5f, -0.025f,\n"
" 0.125f,   -0.5f,  0.025f,\n"
"-0.125f,   -0.5f, -0.025f,\n"
" 0.125f,   -0.5f,  0.025f,\n"
"-0.125f,   -0.5f,  0.025f,\n"
"-0.125f,    0.5f, -0.025f,\n"
" 0.125f,    0.5f, -0.025f,\n"
" 0.125f,    0.5f,  0.025f,\n"
"-0.125f,    0.5f, -0.025f,\n"
" 0.125f,    0.5f,  0.025f,\n"
"-0.125f,    0.5f,  0.025f,\n";

struct vertices vertices_floor = {0};

char data_floor[] =\
"-1.0f,  0.0f, -1.0f,\n"
" 1.0f,  0.0f, -1.0f,\n"
" 1.0f,  0.0f,  1.0f,\n"
 // Second part.
"-1.0f,  0.0f, -1.0f,\n"
" 1.0f,  0.0f,  1.0f,\n"
"-1.0f,  0.0f,  1.0f,\n";

struct v3 {
    GLfloat x;
    GLfloat y;
    GLfloat z;
};

struct v4 {
    GLfloat x;
    GLfloat y;
    GLfloat z;
    GLfloat w;
};

struct v3 v3_sub(struct v3 v1, struct v3 v2)
{
    return (struct v3){v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
}

double v3_magnitude(struct v3 * v)
{
    return sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
}

struct v3 v3_normalize(struct v3 v)
{
    double magnitude = v3_magnitude(&v);
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

double v3_dot(struct v3 * v1, struct v3 * v2)
{
    return v1->x*v2->x + v1->y*v2->y + v1->z*v2->z;
}

double v3_angle(struct v3 * va, struct v3 * vb)
    /*  va \dot vb = ||va|| ||vb|| cos(\omega) ->
     *  cos(\omega) = (va \dot vb) / (||va|| ||vb||) ->
     *  \omega = arccos ((va \dot vb) / (||va|| ||vb||))
     */
{
    double mag_va = v3_magnitude(va);
    double mag_vb = v3_magnitude(vb);

    double dot_product = v3_dot(va, vb);

    double eps = 1e-4;

    double values = dot_product / (mag_va * mag_vb);
    if ((values - eps) < -1) {
        values = -1.0f;
    } else if ((values + eps) > 1) {
        values = 1.0f;
    }
    printf("values: %f\n", values);
    double angle = acos(values);
    printf("angle: %f\n", angle);
    return angle;
}

struct v3 sv3_mul(double scalar, struct v3 * v)
{
    return (struct v3){scalar * v->x, scalar * v->y, scalar * v->z};
}

struct v3 v3_project(struct v3 * v, struct v3 * onto)
    /*  v1 = |v|cos(\omega)*onto_unit */
{
    // Return 0 if velocity is 0.
    if (v->x + v->y + v->z == 0) {
        return (struct v3){0, 0, 0};
    }

    double omega = v3_angle(v, onto);
    printf("omega: %f\n", omega);
    double mag_v = v3_magnitude(v);
    struct v3 onto_unit = v3_normalize(*onto);

    double scalar = mag_v*cos(omega);

    return sv3_mul(scalar, &onto_unit);
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

struct v4 m4_mul_v4(struct m4 * m, struct v4 * v)
{
    GLfloat results[4] = {0};
    GLfloat vector_data[4] = {v->x, v->y, v->z, v->w};
    for (size_t i = 0; i<4; i++) {
        for (size_t j = 0; j<4; j++) {
            results[i] += m->data[i][j] * vector_data[j];
        }
    }
    return (struct v4){results[0], results[1], results[2], results[3]};
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

void m4_translate(struct m4 * m, struct v3 * v_translate)
{
    m->data[0][3] = v_translate->x;
    m->data[1][3] = v_translate->y;
    m->data[2][3] = v_translate->z;
}

void m4_scale(struct m4 * m, struct v3 * v_scale)
{
    m->data[0][0] *= v_scale->x;
    m->data[1][1] *= v_scale->y;
    m->data[2][2] *= v_scale->z;
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

struct bound_box {
    GLfloat data[8][3];
};

struct object {
    struct bound_box bounds;
    struct vertices * vertices;
    struct m4 transformation;
    struct v3 coords;
    struct v3 velocity_x;
    struct v3 velocity_y;
    struct v3 velocity_z;
    struct v3 rotation_velocity;
    struct v3 next_pos;
    struct v3 next_rotation;
    struct v3 scale;
    struct v3 normal;
    struct v3 local_x;
    struct v3 local_z;
    struct v3 rotation;
};

struct bound_box bound_box_get(struct object * object)
{
    struct vertices * vertices = object->vertices;

    GLfloat local_copy[vertices->points];
    memcpy(&local_copy, vertices->data, vertices->size);

    for (size_t i=0; i<vertices->points; i += 3) {
        struct v4 temp_point = {
                                local_copy[i],
                                local_copy[i+1],
                                local_copy[i+2],
                                1.0f
                               };
        struct v4 result = m4_mul_v4(&object->transformation, &temp_point);
        memcpy(&local_copy[i], &result, 3*sizeof(GLfloat));
    }

    float * pointer = local_copy;
    float * end = pointer + vertices->points;

    float x_min = *pointer;
    float x_max = *pointer++;

    float y_min = *pointer;
    float y_max = *pointer++;

    float z_min = *pointer;
    float z_max = *pointer++;

    while (pointer+3 < end) {

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

    return (struct bound_box){{
        {x_min, y_min, z_max}, // bottom front left   #0
        {x_max, y_min, z_max}, // bottom front right  #1
        {x_max, y_max, z_max}, // top front right     #2
        {x_min, y_max, z_max}, // top front left      #3
        {x_min, y_max, z_min}, // top back left       #4
        {x_max, y_max, z_min}, // top back right      #5
        {x_min, y_min, z_min}, // bottom back left    #6
        {x_max, y_min, z_min}, // bottom back right   #7
    }};
}

struct m4 rotation_matrix(float rad_angle, struct v3 * axis)
{
    float s = sinf(rad_angle);
    float c = cosf(rad_angle);

    struct v3 na = v3_normalize(*axis);

    return (struct m4){{
        {c + na.x*na.x*(1 - c),     na.x*na.y*(1 - c) - na.z*s, na.x*na.z*(1-c)+na.y*s,  0.0f},
        {na.y*na.x*(1-c)+na.z*s,    c+na.y*na.y*(1-c),          na.y*na.z*(1-c)-na.x*s,  0.0f},
        {na.z*na.x*(1-c)-na.y*s,    na.z*na.y*(1-c)+na.x*s,     c+na.z*na.z*(1-c),       0.0f},
        {0.0f,                      0.0f,                       0.0f,                    1.0f}
    }};
}


void mv4_rotate(struct v3 * v, float rad_angle, struct v3 * axis)
{
    float result[4] = {0};
    struct m4 m_rot = rotation_matrix(rad_angle, axis);

    float data_buffer[3] = {v->x, v->y, v->z};

    for (size_t i=0; i<4; i++) {
        for (size_t j=0; j<4; j++) {
            result[i] += m_rot.data[i][j] * data_buffer[j];
        }
    }
    v->x = result[0];
    v->y = result[1];
    v->z = result[2];
}


void m4_rotate(struct m4 * m, float rad_angle, struct v3 * axis)
{
    struct m4 m_rot = rotation_matrix(rad_angle, axis);
    struct m4 result = m4_mul(m, &m_rot);
    m4_write(m, &result);
}

void obj_rotate(struct object * object)
{

    struct v3 x_axis = {1.0f, 0.0f, 0.0f};
    struct v3 y_axis = {0.0f, 1.0f, 0.0f};
    struct v3 z_axis = {0.0f, 0.0f, 1.0f};

    // Rotate object.
    m4_rotate(&object->transformation, object->rotation.x, &x_axis);
    m4_rotate(&object->transformation, object->rotation.y, &y_axis);
    m4_rotate(&object->transformation, object->rotation.z, &z_axis);

    // Rotate normal and other vectors.
    mv4_rotate(&object->normal, object->rotation.x, &x_axis);
    mv4_rotate(&object->normal, object->rotation.y, &y_axis);
    mv4_rotate(&object->normal, object->rotation.z, &z_axis);

    mv4_rotate(&object->local_x, object->rotation.x, &x_axis);
    mv4_rotate(&object->local_x, object->rotation.y, &y_axis);
    mv4_rotate(&object->local_x, object->rotation.z, &z_axis);

    mv4_rotate(&object->local_z, object->rotation.x, &x_axis);
    mv4_rotate(&object->local_z, object->rotation.y, &y_axis);
    mv4_rotate(&object->local_z, object->rotation.z, &z_axis);
}

void obj_translate_v3(struct object * object, struct v3 * coords)
{
    // Translate object.
    m4_translate(&object->transformation, coords);
}

void obj_translate(struct object * object)
{
    obj_translate_v3(object, &object->coords);
}

void obj_update_bounds(struct object * object)
{
    object->bounds = bound_box_get(object);
}

void obj_scale(struct object *object)
{
    m4_scale(&object->transformation, &object->scale);
}

struct v3 v3_mul(struct v3 * v1, struct v3 * v2)
{
    return (struct v3){
        v1->x * v2->x,
        v1->y * v2->y,
        v1->z * v2->z,
    };
}

struct v3 v3_scale(struct v3 * v1, struct v3 * v2)
{
    float v2x = fabsf(v2->x);
    float v2y = fabsf(v2->y);
    float v2z = fabsf(v2->z);
    return (struct v3){
        v1->x * v2x,
        v1->y * v2y,
        v1->z * v2z,
    };
}

struct v3 v3_add(struct v3 * v1, struct v3 * v2)
{
    return (struct v3){
        v1->x + v2->x,
        v1->y + v2->y,
        v1->z + v2->z,
    };
}

uint16_t bound_order[] = {
    // First face.
    0, 1,
    1, 2,
    2, 3,
    3, 0,
    // Connect everything "back".
    0, 6,
    1, 7,
    2, 5,
    3, 4,
    // Draw opposite face.
    6, 7,
    7, 5,
    5, 4,
    4, 6
};

struct object floors[3];

void load_vertices(struct vertices * vertices, char * string)
{
    char * saveptr = NULL;
    const char * delimiter = ",";

    char * token = strtok_r(string, delimiter, &saveptr);

    size_t data_size = 512;
    size_t num_points = 0;

    vertices->data = malloc(data_size*sizeof(GLfloat));
    if (!vertices->data) {
        fprintf(stderr, "Could not allocate enough data for loading vertices.\n");
        exit(EXIT_FAILURE);
    }
    GLfloat * last_entry = vertices->data;

    while (token != NULL) {
        *last_entry = strtof(token, NULL);
        last_entry++;
        num_points++;
        if (num_points >= data_size) {
            data_size *= 2;
            vertices->data = realloc(vertices->data, data_size*sizeof(GLfloat));
            if (!vertices->data) {
                fprintf(stderr, "Not enough memory to reallocate data.\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok_r(NULL, delimiter, &saveptr);
    }
    if (num_points < data_size) {
        vertices->data = realloc(vertices->data, num_points*sizeof(GLfloat));
    }
    vertices->size = num_points * sizeof(GLfloat);
    vertices->points = num_points;
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

bool coords_overlap(double min1,
                    double max1,
                    double min2,
                    double max2,
                    float * offset,
                    float o1_velocity,
                    float normal_component)

    /* Check overlap of 4 values.
     *
     * NOTE: The first set of coordinates will currently always be the "active"
     *       object, which means that the distance to move 1 to avoid collision
     *       should be returned.
     *
     * - <--- velocity ---> +
     */
{

    bool pos_velocity = o1_velocity > 0;
    bool neg_velocity = !pos_velocity;

    bool velocity_along_normal = o1_velocity * normal_component > 0;

    // 1: x-----x -->
    // 2:   x-------
    // o:   |---|
    if (min2 >= min1 && min2 <= max1 && pos_velocity) {
        if (velocity_along_normal) {
            *offset = min2 - min1;
        } else {
            *offset = max1 - min2;
        }
        return true;
    }

    // 1: <-- x-----x
    // 2:  -------x
    // o:     |---|
    if (max2 <= max1 && max2 >= min1 && neg_velocity) {
        *offset = max2 - min1;
        return true;
    }

//    // 1: <-- x-------
//    // 2: x-------x
//    // o:     |---|
//    if (min1 >= min2 && min1 <= max2 && neg_velocity) {
//        *offset = max2 - min1;
//        return true;
//    }
//
//    // 1:  -------x
//    // 2:     x-----x
//    // o:     |---|
//    if (max1 <= max2 && max1 >= min2) {
//        *offset = max1 - min2;
//        return true;
//    }

//    // 1: x----------x
//    // 2:   x--x
//    //o1: |----|
//    //o2:   |--------|
//    //   return smallest offset.
//    if (min1 <= min2 && max1 >= max2) {
//        double offset1 = max2 - min1;
//        double offset2 = max1 - min2;
//        if (offset1 < offset2) {
//            *offset = offset1;
//        } else {
//            *offset = offset2;
//        }
//        return true;
//    }

    // 1:- <- x--x -> +
    // 2:   x----------x
    //o1:   |----|
    //o2:     |--------|
    if (min2 <= min1 && max2 >= max1) {
        if (neg_velocity) {
            *offset = max2 - min1;
        } else {
            *offset = max1 - min2;
        }
        return true;
    }

    *offset = 0;
    return false;
}

bool pos_collides(struct object * o1, struct object * o2, struct v3 * offset)
{
    struct bound_box * b1 = &o1->bounds;
    struct bound_box * b2 = &o2->bounds;

    struct v3 * s1 = &o1->scale;
    struct v3 * s2 = &o2->scale;

    struct v3 c1 = o1->next_pos;
    struct v3 c2 = o2->next_pos;

    double xrange1 = (b1->data[1][0] - b1->data[0][0])*0.5f*s1->x;
    double xrange2 = (b2->data[1][0] - b2->data[0][0])*0.5f*s2->x;

    double xmax1 = c1.x + xrange1;
    double xmin1 = c1.x - xrange1;

    double xmax2 = c2.x + xrange2;
    double xmin2 = c2.x - xrange2;

    bool collides_x = coords_overlap(xmin1, xmax1, xmin2, xmax2, &offset->x, o1->velocity_x.x, o2->normal.x);
    if (!collides_x) {
        return false;
    }

    double yrange1 = (b1->data[2][1] - b1->data[0][1])*0.5f*s1->y;
    double yrange2 = (b2->data[2][1] - b2->data[0][1])*0.5f*s2->y;

    double ymax1 = c1.y + yrange1;
    double ymin1 = c1.y - yrange1;

    double ymax2 = c2.y + yrange2;
    double ymin2 = c2.y - yrange2;

    bool collides_y = coords_overlap(ymin1, ymax1, ymin2, ymax2, &offset->y, o1->velocity_y.y, o2->normal.y);
    if (!collides_y) {
        return false;
    }

    double zrange1 = (b1->data[0][2] - b1->data[4][2])*0.5f*s1->z;
    double zrange2 = (b2->data[0][2] - b2->data[4][2])*0.5f*s2->z;

    double zmax1 = c1.z + zrange1;
    double zmin1 = c1.z - zrange1;

    double zmax2 = c2.z + zrange2;
    double zmin2 = c2.z - zrange2;

    bool collides_z = coords_overlap(zmin1, zmax1, zmin2, zmax2, &offset->z, o1->velocity_z.z, o2->normal.z);
    if (!collides_z) {
        return false;
    }

    return true;
}

void obj_update_next_pos(struct object * object)
    /* Generate and store next position in object data. */
{
    struct v3 * vx = &object->velocity_x;
    struct v3 * vy = &object->velocity_y;
    struct v3 * vz = &object->velocity_z;

    object->next_pos.x = object->coords.x + (vx->x+vy->x+vz->x)*time_delta;
    object->next_pos.y = object->coords.y + (vx->y+vy->y+vz->y)*time_delta;
    object->next_pos.z = object->coords.z + (vx->z+vy->z+vz->z)*time_delta;

    object->next_rotation.x = object->rotation.x + object->rotation_velocity.x*time_delta;
    object->next_rotation.y = object->rotation.y + object->rotation_velocity.y*time_delta;
    object->next_rotation.z = object->rotation.z + object->rotation_velocity.z*time_delta;
}

void pos_update(struct object * object)
    /* Determine if object collides with floor, if not, update. */
{
    obj_update_next_pos(object);
    struct v3 offsets = {0};
    for (size_t i = 0; i<SIZE(floors); i++) {
        obj_update_next_pos(&floors[i]);

        if (pos_collides(object, &floors[i], &offsets)) {

            pos_collides(object, &floors[i], &offsets);
            struct v3 * normal = &floors[i].normal;
            struct v3 * local_x = &floors[i].local_x;
            struct v3 * local_z = &floors[i].local_z;
            struct v3 * vx = &object->velocity_x;
            struct v3 * vy = &object->velocity_y;
            struct v3 * vz = &object->velocity_z;

            *vx = v3_project(vx, local_x);
            *vy = v3_project(vy, normal);
            *vz = v3_project(vz, local_z);

            float smallest_offset = offsets.x;
            if (offsets.y < smallest_offset) {
                smallest_offset = offsets.y;
            }
            if (offsets.z < smallest_offset) {
                smallest_offset = offsets.z;
            }

            object->coords.x += normal->x * smallest_offset;
            object->coords.y += normal->y * smallest_offset;
            object->coords.z += normal->z * smallest_offset;
        }
    }
    obj_update_next_pos(object);

    object->coords.x = object->next_pos.x;
    object->coords.y = object->next_pos.y;
    object->coords.z = object->next_pos.z;

    object->rotation.x = fmodf(object->next_rotation.x, 2*M_PI);
    object->rotation.y = fmodf(object->next_rotation.y, 2*M_PI);
    object->rotation.z = fmodf(object->next_rotation.z, 2*M_PI);

    obj_translate_v3(object, &object->next_pos);
}

void object_init(struct object * object)
    /* Initialize object with sane values. */
{
    /* Set scale. */
    object->scale.x = 1.0f;
    object->scale.y = 1.0f;
    object->scale.z = 1.0f;

    /* Set transformation matrix. */
    object->transformation = m4_eye();

    /* Calculate object bounding box.*/
    object->bounds = bound_box_get(object);

    /* Default rotation axis. */
    object->rotation.x = 0.0f;
    object->rotation.y = 0.0f;
    object->rotation.z = 0.0f;

    /* Default normal. */
    object->normal.x = 0.0f;
    object->normal.y = 1.0f;
    object->normal.z = 0.0f;

    /* Default local_x. */
    object->local_x.x = 1.0f;
    object->local_x.y = 0.0f;
    object->local_x.z = 0.0f;

    /* Default local_z. */
    object->local_z.x = 0.0f;
    object->local_z.y = 0.0f;
    object->local_z.z = 1.0f;
}

void construct_box(struct v3 * p_unshifted,
                   struct v3 * x,
                   struct v3 * y,
                   struct v3 * z,
                   GLfloat * box_data)
{
    double half_mag_x = v3_magnitude(x)*0.5f;
    double half_mag_y = v3_magnitude(y)*0.5f;
    double half_mag_z = v3_magnitude(z)*0.5f;

    // Shift p, center around origo.
    struct v3 p = v3_add(p_unshifted, &(struct v3){-half_mag_x,
                                                   -half_mag_y,
                                                   -half_mag_z});

    // Constructing the 'front' face.
    struct v3 f_bottom_left = v3_add(&p, &(struct v3){0,0,0});
    struct v3 f_bottom_right = v3_add(&f_bottom_left,x);
    struct v3 f_top_right = v3_add(&f_bottom_right,y);
    struct v3 f_top_left = v3_sub(f_top_right,*x);
    // Constructing the 'back' face.
    struct v3 b_bottom_left = v3_add(&p, z);
    struct v3 b_bottom_right = v3_add(&b_bottom_left,x);
    struct v3 b_top_right = v3_add(&b_bottom_right,y);
    struct v3 b_top_left = v3_sub(b_top_right,*x);
    // Set up data structure for copy.
    struct v3 temp_data[] = {
        // Front face.
        f_bottom_left,
        f_bottom_right,
        f_top_right,

        f_bottom_left,
        f_top_right,
        f_top_left,

        // Back face.
        b_bottom_left,
        b_bottom_right,
        b_top_right,

        b_bottom_left,
        b_top_right,
        b_top_left,

        // Right face.
        f_bottom_right,
        b_bottom_right,
        b_top_right,

        f_bottom_right,
        b_top_right,
        f_top_right,

        // Left face.
        f_bottom_left,
        b_bottom_left,
        b_top_left,

        f_bottom_left,
        b_top_left,
        f_top_left,

        // Top side.
        f_top_left,
        f_top_right,
        b_top_right,

        f_top_left,
        b_top_right,
        b_top_left,

        // Bottom side.
        f_bottom_left,
        f_bottom_right,
        b_bottom_right,

        f_bottom_left,
        b_bottom_right,
        b_bottom_left,
    };

    memcpy(box_data, temp_data, sizeof(GLfloat)*6*6*3);
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

    // Load cube data into vertices structs.
//    size_t size_data_cube = sizeof(data_cube);
//    char * dynamic_data_cube = malloc(size_data_cube);
//    memcpy(dynamic_data_cube, data_cube, size_data_cube);
    GLfloat dynamic_data_cube[6*6*3];
    construct_box(&(struct v3){0,0,0},
                  &(struct v3){0.3,0,0},
                  &(struct v3){0,0.7,0},
                  &(struct v3){0,0,0.1},
                  dynamic_data_cube);

    vertices_cube.data = &dynamic_data_cube[0];
    vertices_cube.size = sizeof(dynamic_data_cube);
    vertices_cube.points = 6*6*3;
    vertices_cube.vertices = vertices_cube.points/3;

//    load_vertices(&vertices_cube, dynamic_data_cube);
    struct object obj_cube = {0};
    obj_cube.vertices = &vertices_cube;
    object_init(&obj_cube);

    // Load floor data into vertices structs.
    size_t size_data_floor = sizeof(data_floor);
    char * dynamic_data_floor = malloc(size_data_floor);
    memcpy(dynamic_data_floor, data_floor, size_data_floor);
    load_vertices(&vertices_floor, dynamic_data_floor);
    floors[0].vertices = &vertices_floor;
    object_init(&floors[0]);
    floors[0].coords.y = -2.0f;
    floors[0].scale.x = 4.0f;

    floors[0].transformation = m4_eye();
    obj_translate(&floors[0]);
    obj_scale(&floors[0]);
    pos_update(&floors[0]);

    // Make second floor tile.
    floors[1].vertices = &vertices_floor;
    floors[1].coords.y = -2.5f;
    floors[1].coords.z = -20.0f;
    floors[1].scale.x = 4.0f;
    object_init(&floors[1]);

    floors[1].transformation = m4_eye();
    obj_translate(&floors[1]);
    obj_scale(&floors[1]);
    pos_update(&floors[1]);

    // Make third floor tile.
    floors[2].vertices = &vertices_floor;
    object_init(&floors[2]);
    floors[2].coords.y = -2.0f;
    floors[2].coords.z = -2.0f;
    floors[2].coords.x =  2.0f;
    floors[2].scale.x = 4.0f;
    floors[2].rotation.z = M_PI * 0.25;

    floors[2].transformation = m4_eye();
//   floors[2].normal.x = -1.0f;
//   floors[2].normal.y = 1.0f;
    obj_rotate(&floors[2]);
    obj_translate(&floors[2]);
    obj_scale(&floors[2]);
    pos_update(&floors[2]);
    obj_update_bounds(&floors[2]);

    // Set up cube.
    GLuint VBO_cube, VAO_cube;
    glGenBuffers(1, &VBO_cube);
    glGenVertexArrays(1, &VAO_cube);

    glBindVertexArray(VAO_cube);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_cube);
    glBufferData(GL_ARRAY_BUFFER, vertices_cube.size, vertices_cube.data, GL_STATIC_DRAW);

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
    glBufferData(GL_ARRAY_BUFFER, vertices_floor.size, vertices_floor.data, GL_STATIC_DRAW);

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

    glfwSwapInterval(0);

    double time_prev = glfwGetTime();
    double time_current = 0;

    struct m4 mat_projection = m4_perspective(0.1f, 100.0f, M_PI*0.5f, (double)WIDTH/(double)HEIGHT);

    /* Generate temp buffer. */
    GLuint VBO_temp, VAO_temp;
    glGenBuffers(1, &VBO_temp);
    glGenVertexArrays(1, &VAO_temp);

    float * current_rotation_velocity = &obj_cube.rotation_velocity.z;

    while(!glfwWindowShouldClose(window)) {

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        GLint uniform_model = glGetUniformLocation(program_shader_simple, "model");
        GLint uniform_view = glGetUniformLocation(program_shader_simple, "view");
        GLint uniform_projection = glGetUniformLocation(program_shader_simple, "projection");
        GLint uniform_color = glGetUniformLocation(program_shader_simple, "uniform_color");

        struct m4 mat_view = m4_eye();
        m4_translate(&mat_view, &(struct v3){0.0f, 1.0f, -3.0f});

        // Set up time delta.
        time_current = glfwGetTime();
        time_delta = time_current - time_prev;
        time_prev = time_current;

        // Check keys.
        if (key_down[GLFW_KEY_ESCAPE]) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        if (key_down[GLFW_KEY_SPACE]) {
            obj_cube.velocity_y.y += (10.0f + GRAVITY) * time_delta;
        }

        if (key_down[GLFW_KEY_M]) {
            obj_cube.coords.y = 0.0f;
        }

        float rotation_speed = 2.0f * time_delta;

        if (key_down[GLFW_KEY_LEFT]) {
            *current_rotation_velocity -= rotation_speed;
        }
        if (key_down[GLFW_KEY_RIGHT]) {
            *current_rotation_velocity += rotation_speed;
        }

        if (key_down[GLFW_KEY_TAB]) {
            if (*current_rotation_velocity == obj_cube.rotation_velocity.x) {
                current_rotation_velocity = &obj_cube.rotation_velocity.y;
            } else if (*current_rotation_velocity == obj_cube.rotation_velocity.y) {
                current_rotation_velocity = &obj_cube.rotation_velocity.z;
            } else if (*current_rotation_velocity == obj_cube.rotation_velocity.z) {
                current_rotation_velocity = &obj_cube.rotation_velocity.x;
            }
        }

        double val_delta_speed = val_cube_speed * time_delta;
        double val_delta_friction = val_cube_friction * time_delta;
        double next_velocity_z = 0;
        if (key_down[GLFW_KEY_W]) {
            next_velocity_z = obj_cube.velocity_z.z - val_delta_speed;
        } else if (key_down[GLFW_KEY_S]) {
            next_velocity_z = obj_cube.velocity_z.z + val_delta_speed;
        } else {
            double velocity_new_z = 0;
            double velocity_current_z = obj_cube.velocity_z.z;
            if (velocity_current_z > 0) {
                velocity_new_z = velocity_current_z - val_delta_friction;
            } else if (velocity_current_z < 0) {
                velocity_new_z = velocity_current_z + val_delta_friction;
            }
            if (velocity_new_z * velocity_current_z < 0) { // Changed sign.
                velocity_new_z = 0;
            }
            next_velocity_z = velocity_new_z;
        }
        if (next_velocity_z < -val_cube_max_speed) {
            next_velocity_z = -val_cube_max_speed;
        } else if (next_velocity_z > val_cube_max_speed) {
            next_velocity_z = val_cube_max_speed;
        }
        obj_cube.velocity_z.z = next_velocity_z;

        double next_velocity_x = 0;
        if (key_down[GLFW_KEY_D]) {
            next_velocity_x = obj_cube.velocity_x.x + val_delta_speed;
        } else if (key_down[GLFW_KEY_A]) {
            next_velocity_x = obj_cube.velocity_x.x - val_delta_speed;
        } else {
            double velocity_new_x = 0;
            double velocity_current_x = obj_cube.velocity_x.x;
            if (velocity_current_x > 0) {
                velocity_new_x = velocity_current_x - val_delta_friction;
            } else if (velocity_current_x < 0) {
                velocity_new_x = velocity_current_x + val_delta_friction;
            }
            if (velocity_new_x * velocity_current_x < 0) { // Changed sign.
                velocity_new_x = 0;
            }
            next_velocity_x = velocity_new_x;
        }
        if (next_velocity_x < -val_cube_max_speed) {
            next_velocity_x = -val_cube_max_speed;
        } else if (next_velocity_x > val_cube_max_speed) {
            next_velocity_x = val_cube_max_speed;
        }
        obj_cube.velocity_x.x = next_velocity_x;

        obj_cube.velocity_y.y -= GRAVITY * time_delta;

        obj_cube.transformation = m4_eye();
        obj_scale(&obj_cube);
        obj_rotate(&obj_cube);
        obj_translate(&obj_cube);
        obj_update_bounds(&obj_cube);

        pos_update(&obj_cube);

        glUniformMatrix4fv(uniform_model, 1, TRANSPOSE, DATm(obj_cube.transformation));
        glUniformMatrix4fv(uniform_view, 1, TRANSPOSE, DATm(mat_view));
        glUniformMatrix4fv(uniform_projection, 1, TRANSPOSE, DATm(mat_projection));

        glBindVertexArray(VAO_cube);
        glUniform4fv(uniform_color, 1, DATv(color_cube));
        glDrawArrays(GL_TRIANGLES, 0, vertices_cube.vertices);

        glBindVertexArray(VAO_floor);

        for (size_t i = 0; i<SIZE(floors); i++) {
            glUniformMatrix4fv(uniform_model, 1, TRANSPOSE, DATm(floors[i].transformation));
            glUniform4fv(uniform_color, 1, DATv(color_floor));
            glDrawArrays(GL_TRIANGLES, 0, floors[i].vertices->size);
        }

        glBindVertexArray(VAO_temp);

        GLfloat cube_wireframe_data[SIZE(bound_order)*3];

        for (size_t i=0; i<SIZE(bound_order); i += 2) {

            int first = bound_order[i];
            int second = bound_order[i+1];

            memcpy(&cube_wireframe_data[3*i], obj_cube.bounds.data[first], 3*sizeof(GLfloat));
            memcpy(&cube_wireframe_data[3*(i+1)], obj_cube.bounds.data[second], 3*sizeof(GLfloat));
        }

        struct m4 eye = m4_eye();
        glUniformMatrix4fv(uniform_model, 1, TRANSPOSE, DATm(eye));
        glUniformMatrix4fv(uniform_view, 1, TRANSPOSE, DATm(mat_view));
        glUniformMatrix4fv(uniform_projection, 1, TRANSPOSE, DATm(mat_projection));
        GLfloat color_line[] = {0.0f, 1.0f, 0.0f, 1.0f};
        glUniform4fv(uniform_color, 1, DATv(color_line));

        glBindBuffer(GL_ARRAY_BUFFER, VBO_temp);
        glBufferData(GL_ARRAY_BUFFER, SIZE(bound_order)*3*sizeof(GLfloat), cube_wireframe_data, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 3*sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);
        glDrawArrays(GL_LINES, 0, SIZE(bound_order));

        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    // Terminate glfw.
    glfwTerminate();

    // Free object data.
//    free(dynamic_data_cube);
    free(dynamic_data_floor);

    return 0;
    // check:  http://www.dyn4j.org/2010/01/sat/ for SAT
}
