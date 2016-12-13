#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdbool.h>
#include <GLFW/glfw3.h>
#include "maths/math_utils.h"
#include "globals/globals.h"

typedef enum event_action_type {
    PASSTHROUGH,
    BLOCKING,
} event_action_type;

typedef enum comparison_type {
    GT,
    GTEQ,
    LT,
    LTEQ,
    EQ,
} comparison_type;

enum uniform_type {
    /* Strip the gl part of the glUniform* functions used. */
    UniformMatrix4fv,
    Uniform1f,
    NUM_TYPES,
};

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

typedef float (*mod_function)(float current, float input, void * data);

typedef struct Command_Packet {
    float * variable;
    float value;
    int lifetime;
    event_action_type type;
    mod_function mod_function;
    void * data;
    struct Command_Packet * next;
    struct Command_Packet * sub_commands;
    struct Command_Packet * last_sub;
} Command_Packet;

typedef struct Command_Input {
    float * variable;
    float value;
    mod_function mod_function;
    void * data;
    int lifetime;
    event_action_type type;
} Command_Input;

typedef bool (*logic_function)(float left_side,
                               comparison_type comp,
                               float right_side);

typedef struct Action_Logic_Data{
    // Logic comparison data.
    logic_function logic_function;
    float comparison_value;
    comparison_type comparison_type;
    // Regular function to use if logic does not trigger.
    mod_function regular_function;
    // Alternative function that is run if the logic triggers.
    mod_function replacement_function;
    float replacement_value;
} Action_Logic_Data;

struct component {
    char * id;
    m4 transformation;
    VAO * vao;
    float modifiers[3];
    Command_Packet * command_list;
    Command_Packet * last_command;
    struct component * next;
    uint64_t flags;
};

struct uniform_data {
    const char * name;
    void * (*data_function)(struct component *);
    enum uniform_type type;
};

struct collision_bound_data {
    float bound;
    float value;
    comparison_type compare_type;
    void (*flag_operation)(struct component * component, enum flag_type flag);
    enum flag_type flag;
};

#endif
