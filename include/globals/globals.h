#ifndef GLOBALS_H
#define GLOBALS_H

#include <GLFW/glfw3.h>

#define GRAVITY 0.03f

typedef enum global_index {
    gravity,
    speed,
    is_jumping,
    glfw_time,
    num_constants,
} global_index;

extern float global_variables[num_constants];

void setup_globals(void);

#endif
