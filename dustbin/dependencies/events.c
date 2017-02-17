#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <GLFW/glfw3.h>
#include <math.h>

#include "globals/globals.h"
#include "events/events.h"
#include "utils/utils.h"
#include "components/components.h"

// ### Prototypes for functions further down in the document.

bool keymap[NUM_KEYS] = {false};
uint32_t num_actions[NUM_KEYS] = {0};

void process_keys(GLFWwindow * window);
void collision_check(struct component * component, struct component * other);
void get_current_coordinates(struct component * component, v3 * result);

void process_command_list(
                          float * modifiers,
                          struct Command_Packet ** command_list,
                          struct Command_Packet ** last,
                          int current_level
                         );

// ### End prototypes.

void callback_key(
                  GLFWwindow * window,
                  int key,
                  int scancode,
                  int action,
                  int mods
                 )
{
    if (action == GLFW_PRESS) {
        keymap[key] = true;
    } else if (action == GLFW_RELEASE) {
        keymap[key] = false;
        num_actions[key] = 0;
    }
}

void poll_events(GLFWwindow * window) {
    glfwPollEvents();
//    process_keys(window);
}
