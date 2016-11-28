
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

float global_constants[num_constants];

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
void setup_globals();
