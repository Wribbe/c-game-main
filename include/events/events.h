
#include "maths/math_utils.h"
#include "utils/utils.h"

#define NUM_KEYS 512

#define X 0
#define Y 1
#define Z 2

int * keymap;

VAO global_vao;

m4 transformation;

void callback_key(
                  GLFWwindow * window,
                  int key,
                  int scancode,
                  int action,
                  int mods
                 );

void poll_events(GLFWwindow * window);
