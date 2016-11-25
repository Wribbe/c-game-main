#include <stdio.h>
#include <stdlib.h>

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

// ### Prototypes for functions further down in the document.

void process_keys(GLFWwindow * window);
void collision_check(
                     float * transformation_matrix,
                     float * x_modifier,
                     float * x_write_pos,
                     float * y_modifier,
                     float * y_write_pos
                    );

void process_command_list(float * modifiers);

// ### End prototypes.

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


Command_Packet * command_list = NULL;
Command_Packet * last = NULL;

void submit_command(int index, float value, unsigned int lifetime)
{
    /* Submit Command Packet based on input values. */

    // Create new Command_Packet.
    Command_Packet * new_command = malloc(sizeof(Command_Packet));

    // Populate Command_Packet.
    new_command->index = index;
    new_command->value = value;
    new_command->lifetime = lifetime;
    new_command->next = NULL;

    if (command_list == NULL) { // Begin new command_list.
        command_list = new_command;
        last = new_command;
    } else { // Append to end of current list.
        // Append to last element.
        last->next = new_command;
        // Move last pointer;
        last = new_command;
    }
}


void process_keys(GLFWwindow * window)
{
    if (check(GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    // Initialize modifiers.
    float modifiers[3] = {0};

    float * x_modifier = &modifiers[X];
    float * y_modifier = &modifiers[Y];

    float speed = 0.05f;
    float gravity = 0.03f;

    float * x_write_pos = &transformation[0][3];
    float * y_write_pos = &transformation[1][3];

    // Modify positions along x-axis.
    if (check(GLFW_KEY_LEFT)) {
        submit_command(X, -speed, 1);
    }
    if (check(GLFW_KEY_RIGHT)) {
        submit_command(X, +speed, 1);
    }

    // Gravitation.
    *y_modifier -= gravity;

    // Process command list.
    process_command_list(modifiers);

    // Check collisions with the window.
    collision_check(transformation,
                    x_modifier,
                    x_write_pos,
                    y_modifier,
                    y_write_pos);

    // Write any modified data to the transformation matrix.
    *x_write_pos += *x_modifier;
    *y_write_pos += *y_modifier;
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

void process_command_list(float * modifiers)
{
    // Process the command list.
    Command_Packet * old_pointer = NULL;
    Command_Packet * current_pointer = command_list;
    Command_Packet * next_pointer = NULL;

    // Temporary pointer for removal tagging.
    Command_Packet * temp = NULL;

    // Iterate over the command list.
    while(current_pointer != NULL) {

        // Assign the next pointer.
        next_pointer = current_pointer->next;
        // Apply the modifications in the package.
        modifiers[current_pointer->index] += current_pointer->value;
        // Decrease lifetime by one.
        current_pointer->lifetime -= 1;

        // Three types of unlink scenarios:
        //
        //      1.) [..]->o->c->n->[..]
        //      2.) c->n->[..]
        //      3.) [..]->o->c->N
        //
        //  Legend:
        //      o - old_pointer.
        //      c - current_pointer.
        //      n - current_pointer->next.
        //      N - NULL.
        //
        //  Where c is the current node with lifetime == 0;
        //      1, The c node is in the middle of the list.
        //      2, The c node is at the beginning of the list.
        //      3, The c node is at the end of the list.
        //          3.1.) End of list with predecessor.
        //          3.2.) Only element in list.
        //
        //  Detection:
        //      1.) current node has a next node and the old_pointer is not
        //          NULL.
        //      2.) current->next is not NULL, but the old_pointer is NULL.
        //      3.) current->next is NULL.
        //          3.1.) old_pointer is not NULL.
        //          3.2.) old_pointer is NULL.
        //
        //  Actions for unlinking:
        //      1.) Point old_pointer->next to next_pointer.
        //          old_pointer is still the old pointer, next becomes the
        //          current, and new next_pointer = new current->next;
        //      2.) Assign command_list to current_pointer->next, keep
        //          old_pointer as NULL.
        //      3.)
        //          3.1) Assign old_pointer->next = NULL, move global last
        //               pointer.
        //          3.2) Assign command_list = NULL, and set last = NULL.
        //

        if (current_pointer->lifetime == 0) {
            // Unlink dead command.

            // Save the current pointer in temp.
            temp = current_pointer;

            // Store logical values.
            int old_pointer_assigned = old_pointer != NULL;
            int current_has_next = current_pointer->next != NULL;

            if (old_pointer_assigned && current_has_next) {
                // In the middle of the list, see 1 above.
                // Re-link old->next to next.
                old_pointer->next = next_pointer;
                // Advance the current pointer.
                current_pointer = next_pointer;
                // Advance the next pointer.
                next_pointer = current_pointer->next;
            } else if (!old_pointer_assigned && current_has_next) {
                // At the first element of the list, see 2 above.
                command_list = current_pointer->next;
                // Advance the current_pointer.
                current_pointer = command_list;
                // Check if there is something bound to current_pointer.
                if (current_pointer != NULL) {
                    next_pointer = current_pointer->next;
                }
            } else if (!current_has_next) { // End of list or only element.
                if (old_pointer_assigned) { // Last with predecessor.
                    // At the last element of the list, see 3.1 above.
                    old_pointer->next = NULL;
                    // Re assign last node pointer.
                    last = old_pointer;
                } else { // Only element in list.
                    command_list = NULL;
                    last = NULL;
                }
                // Set the current_pointer to NULL.
                current_pointer = NULL;
            } else {
                fprintf(stderr, "Unknown command list state, should not be here, aborting.\n");
                exit(1);
            }
            // De-allocate the temp pointer.
            free(temp);
            temp = NULL;
        }
    }
}
