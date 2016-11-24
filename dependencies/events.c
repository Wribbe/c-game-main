#include <stdio.h>

#include "GLFW/glfw3.h"
#include "events/events.h"

int local_keymap[NUM_KEYS];
int * keymap = local_keymap;

// Control single object.
m4 transformation = {
    {1.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 1.0f},
};

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
}

void poll_events(GLFWwindow * window) {
    glfwPollEvents();
    process_keys(window);
}

int check(GLuint key) {
    return keymap[key];
}

float speed = 0.05f;

void process_keys(GLFWwindow * window)
{
    if (check(GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (check(GLFW_KEY_LEFT)) {
        transformation[0][3] -= speed;
    }
    if (check(GLFW_KEY_RIGHT)) {
        transformation[0][3] += speed;
    }
    if (check(GLFW_KEY_UP)) {
        transformation[1][3] += speed;
    }
    if (check(GLFW_KEY_DOWN)) {
        transformation[1][3] -= speed;
    }
}
