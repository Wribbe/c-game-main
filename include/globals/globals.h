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

enum flag_type {
    JUMPING,
    GRAVITY_ON,
};

#endif
