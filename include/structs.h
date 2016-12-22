#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdbool.h>
#include <GLFW/glfw3.h>
#include "maths/math_utils.h"
#include "globals/globals.h"

typedef struct Point_Data {
    size_t rows;
    size_t elements;
    float * data;
} Point_Data;

typedef struct Attrib_Pointer_Info {
    GLuint index;
    GLint size;
    float * offset;
} Attrib_Pointer_Info;

typedef struct VBO {
    GLuint vbo;
    GLuint render_geometry;
    GLuint draw_type;
    Point_Data * point_data;
} VBO;

typedef struct VAO {
    VBO * vbo;
    GLuint vao;
    GLint start;
    GLsizei count;
    v3 bounds[8];
    GLsizei list_size;
    Attrib_Pointer_Info * attrib_list;
} VAO;

struct component {
    m4 transformation;
    size_t uniform_size;
    VAO * vao;
    struct uniform_data * uniform_data;
    struct component * next;
};

#endif
