
#include "maths/math_utils.h"

#define NUM_KEYS 512

int * keymap;

m4 transformation;

void callback_key(
                  GLFWwindow * window,
                  int key,
                  int scancode,
                  int action,
                  int mods
                 );

void poll_events(GLFWwindow * window);
