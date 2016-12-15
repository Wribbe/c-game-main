#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdbool.h>
#include <GLFW/glfw3.h>
#include "maths/math_utils.h"
#include "globals/globals.h"

// Forward declaration of submit_type for wrapper function.
union submit_type;
struct Command_Packet;
struct component;
struct s_flag;
struct s_float;

enum event_action_type {
    PASSTHROUGH,
    BLOCKING,
} event_action_type;

enum uniform_type {
    /* Strip the gl part of the glUniform* functions used. */
    UniformMatrix4fv,
    Uniform1f,
    NUM_TYPES,
};

typedef enum comparison_type {
    GT,
    GTEQ,
    LT,
    LTEQ,
    EQ,
} comparison_type;

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

typedef float (*mod_function)(float current, float input, void * data);

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

struct component {
    char * id;
    m4 transformation;
    VAO * vao;
    float modifiers[3];
    struct Command_Packet * command_list;
    struct Command_Packet * last_command;
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

struct s_flag {
    // Begin manual input.
    void (*function)(struct component * component, enum flag_type flag);
    enum flag_type flag;
    struct component * component;
    int lifetime;
    enum event_action_type action_type;
    // End manual input.
};

struct s_float {
    // Begin manual input.
    float (*function)(float input, float * value, void * data);
    float input;
    float * variable;
    int lifetime;
    void * data;
    enum event_action_type action_type;
    // End manual input.
    float * result;
    bool * signal_result;
    float ** result_write_pos;
};

union submit_type {
    struct s_flag s_flag;
    struct s_float s_float;
};

struct Command_Packet {
    // Standard variables for all command.s
    int lifetime;
    enum event_action_type action_type;
    // Special flag and result store for float commands.
    bool got_result;
    float result;
    float * variable_write_pos;
    // Pointers that make it possible to link Command_Packets together.
    struct Command_Packet * next;
    struct Command_Packet * sub_commands;
    struct Command_Packet * last_sub;
    // Standard wrapper and remove functionality for submit_types.
    void (*wrapper_function)(union submit_type * type);
    void (*remove)(union submit_type * type);
    // The submit_type itself.
    union submit_type type;
};

extern void free_s_flag(union submit_type * type);
extern void free_s_float(union submit_type * type);

#endif
