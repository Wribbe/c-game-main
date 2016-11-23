#ifndef STRUCTS_H
#define STRUCTS_H

#include <GLFW/glfw3.h>

typedef struct {
    int rows;
    int elements;
    void * data;
} Point_Data;

typedef struct {
    GLuint index;
    GLint size;
    GLsizei stride;
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
    GLsizei list_size;
    Attrib_Pointer_Info * attrib_list;
} VAO;

#endif
