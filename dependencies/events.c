#include <stdio.h>

#include "GLFW/glfw3.h"
#include "events/events.h"

int local_keymap[NUM_KEYS];
int * keymap = local_keymap;


void process_keys(GLFWwindow * window);


void callback_key(
                  GLFWwindow * window,
                  int key,
                  int scancode,
                  int action,
                  int mods
                 )
{
    if (action == GLFW_PRESS && !keymap[key]) {
        keymap[key] = 1;
    } else if (action == GLFW_RELEASE && keymap[key]) {
        keymap[key] = 0;
    }
    process_keys(window);
}

int check(GLuint key) {
    return keymap[key];
}

void process_keys(GLFWwindow * window)
{
    if (check(GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (check(GLFW_KEY_LEFT)) {
        printf("LEFT!\n");
    }
    if (check(GLFW_KEY_RIGHT)) {
        printf("RIGHT!\n");
    }
    if (check(GLFW_KEY_UP)) {
        printf("UP!\n");
    }
    if (check(GLFW_KEY_DOWN)) {
        printf("DOWN!\n");
    }
}
