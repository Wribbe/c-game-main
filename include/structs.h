#ifndef STRUCTS_H
#define STRUCTS_H

#include <GLFW/glfw3.h>

typedef struct {
    size_t rows;
    size_t elements;
    void * data;
} Point_Data;

typedef struct {
    GLuint index;
    GLint size;
    float * offset;
} Attrib_Pointer_Info;

typedef struct {
    GLuint vbo;
    GLuint render_geometry;
    GLuint draw_type;
    Point_Data * point_data;
} VBO;

typedef struct {
    VBO vbo;
    GLuint vao;
    GLint start;
    GLsizei count;
    GLfloat bounds[24];
    GLsizei list_size;
    Attrib_Pointer_Info * attrib_list;
} VAO;

#endif
