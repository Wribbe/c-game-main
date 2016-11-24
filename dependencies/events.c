#include <stdio.h>

#include "GLFW/glfw3.h"
#include "events/events.h"

int local_keymap[NUM_KEYS];
int * keymap = local_keymap;

//Control single object.
m4 transformation = {
    {0.6f, 0.0f, 0.0f, 0.0f},
    {0.0f, 0.5f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.5f, 0.0f},
    {0.0f, 0.0f, 0.0f, 1.0f},
};

void process_keys(GLFWwindow * window);
void collision_check(
                     float * transformation_matrix,
                     float * x_modifier,
                     float * x_write_pos,
                     float * y_modifier,
                     float * y_write_pos
                    );

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

    collision_check(transformation,
                    &x_modifier,
                    x_write_pos,
                    &y_modifier,
                    y_write_pos);

    *x_write_pos += x_modifier;
    *y_write_pos += y_modifier;
}

void collision_check(
                     float * transformation_matrix,
                     float * x_modifier,
                     float * x_write_pos,
                     float * y_modifier,
                     float * y_write_pos
                    )
{
    /* Check collisions against window border. If collision detected set the
     * position of the object at the edge of the window bounding box.
     */

    float x_pos_border =  1.0f;
    float x_neg_border = -1.0f;
    float y_pos_border =  1.0f;
    float y_neg_border = -1.0f;

    float * bounds = global_vao.bounds;

    // Get height and widht of bounding box.
    float width = bounds[1*3+0] - bounds[0*3+0];  // 1'st x - 0'th x.
    float height = bounds[0*3+1] - bounds[2*3+1]; // 0'th y - 2'nd y.
    float depth = bounds[4*3+3] - bounds[0*3+3];  // 4'th z - 0'th z.

    // Correctly handle scaling boxes.
    float x_scale = transformation[0][0];
    float y_scale = transformation[1][1];
    float z_scale = transformation[2][2];

    // Scale width and height;
    width *= x_scale;
    height *= y_scale;
    depth *= z_scale;

    // Calculate next position for x and y.
    float next_x = *x_write_pos + *x_modifier;
    float next_y = *y_write_pos + *y_modifier;

    float half_width = width/2.0f;
    float half_height = height/2.0f;

    float pos_bound_x_val = next_x + half_width;
    float neg_bound_x_val = next_x - half_width;

    float pos_bound_y_val = next_y + half_height;
    float neg_bound_y_val = next_y - half_height;

    // Check out of bounds x.
    if (pos_bound_x_val >= x_pos_border) {
        *x_modifier = 0;
        *x_write_pos = x_pos_border - half_width;
    } else if (neg_bound_x_val <= x_neg_border) {
        *x_modifier = 0;
        *x_write_pos = x_neg_border + half_width;
    }

    // Check out of bounds y.
    if (pos_bound_y_val >= y_pos_border) {
        *y_modifier = 0;
        *y_write_pos = y_pos_border - half_height;
    } else if (neg_bound_y_val <= y_neg_border) {
        *y_modifier = 0;
        *y_write_pos = y_neg_border + half_height;
    }
}
