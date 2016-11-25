#ifndef STRUCTS_H
#define STRUCTS_H

#include <GLFW/glfw3.h>

typedef struct Point_Data {
    size_t rows;
    size_t elements;
    void * data;
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
    VBO vbo;
    GLuint vao;
    GLint start;
    GLsizei count;
    GLfloat bounds[24];
    GLsizei list_size;
    Attrib_Pointer_Info * attrib_list;
} VAO;

typedef struct Command_Packet {
    int index;
    float value;
    unsigned int lifetime;
    struct Command_Packet * next;
} Command_Packet;

#endif
