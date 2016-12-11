#ifndef EVENTS_H
#define EVENTS_H

#include "maths/math_utils.h"
#include "utils/utils.h"

#define NUM_KEYS 512

#define X 0
#define Y 1
#define Z 2

typedef enum global_index {
    gravity,
    speed,
    is_jumping,
    num_constants,
} global_index;

float global_variables[num_constants];

int * keymap;

struct component * component_list;
struct component * current_component;

void callback_key(
                  GLFWwindow * window,
                  int key,
                  int scancode,
                  int action,
                  int mods
                 );

void poll_events(GLFWwindow * window);
void setup_globals();

#endif
