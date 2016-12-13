#ifndef EVENTS_H
#define EVENTS_H

#include <stdbool.h>

#include "maths/math_utils.h"
#include "utils/utils.h"

#define X 0
#define Y 1
#define Z 2

#define NUM_KEYS 512

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
void setup_globals();

#endif
