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


void process_keys(GLFWwindow * window)
{
    if (check(GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    float x_modifier = 0;
    float y_modifier = 0;

    float speed = 0.05f;

    float * x_write_pos = &transformation[0][3];
    float * y_write_pos = &transformation[1][3];

    // Modify positions along x-axis.
    if (check(GLFW_KEY_LEFT)) {
        x_modifier -= speed;
    }
    if (check(GLFW_KEY_RIGHT)) {
        x_modifier += speed;
    }

    // Modify positions along y-axis.
    if (check(GLFW_KEY_UP)) {
        y_modifier += speed;
    }
    if (check(GLFW_KEY_DOWN)) {
        y_modifier -= speed;
    }

    float x_pos_border =  1.0f;
    float x_neg_border = -1.0f;
    float y_pos_border =  1.0f;
    float y_neg_border = -1.0f;

    float * bounds = global_vao.bounds;

    // Get height and widht of bounding box.
    float width = bounds[3] - bounds[0];
    float heigth = bounds[4] - bounds[1];

    // Calcutlate next position for x and y.
    float next_x = *x_write_pos + x_modifier;
    float next_y = *y_write_pos + y_modifier;

    // Check out of bounds x.
    if (!(next_x + width/2 > x_pos_border) && !(next_x - width/2 < x_neg_border)) {
        *x_write_pos += x_modifier;
    }

    // Check out of bounds y.
    if (!(next_y + width/2 > y_pos_border) && !(next_y - width/2 < y_neg_border)) {
        *y_write_pos += y_modifier;
    }
}
