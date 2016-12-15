#ifndef GLOBALS_H
#define GLOBALS_H

#include <GLFW/glfw3.h>

#define NUM_KEYS 512
#define GRAVITY 0.03f

enum coord {
    X,
    Y,
    Z,
};

typedef enum global_index {
    gravity,
    speed,
    is_jumping,
    glfw_time,
    wb_x_pos,
    wb_x_neg,
    wb_y_pos,
    wb_y_neg,
    // Keep this one as last element.
    num_globals,
} global_index;

extern float global_variables[];

void setup_globals(void);
void global_init(void);

enum flag_type {
    JUMPING,
    GRAVITY_ON,
};

enum component_list_type {
    CONTROLLABLE,
    NON_CONTROLLABLE,
    NUM_COMPONENT_TYPES,
};

// Global pointers to different components.
struct component * components[NUM_COMPONENT_TYPES];
struct component * last_component[NUM_COMPONENT_TYPES];
struct component * controlled_component;

#endif
