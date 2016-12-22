#ifndef EVENTS_H
#define EVENTS_H

#include <stdbool.h>

#include "maths/math_utils.h"
#include "utils/utils.h"
#include "globals/globals.h"

extern bool keymap[NUM_KEYS];
extern bool release[NUM_KEYS];

void callback_key(
                  GLFWwindow * window,
                  int key,
                  int scancode,
                  int action,
                  int mods
                 );

void poll_events(GLFWwindow * window);
void collision_keep_outside_border(enum coord coord,
                                   struct component * component,
                                   struct component * other,
                                   enum bound_order bound_order);
void collision_keep_inside_border(enum coord coord,
                                  struct component * component,
                                  struct component * other);
#endif
