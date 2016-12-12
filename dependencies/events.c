#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "GLFW/glfw3.h"
#include "events/events.h"
#include "utils/utils.h"
#include "components/components.h"

#define GRAVITY 0.03f

// ### Prototypes for functions further down in the document.

bool keymap[NUM_KEYS] = {false};
uint32_t num_actions[NUM_KEYS] = {0};

void process_keys(GLFWwindow * window);
void collision_check(
                     float * transformation_matrix,
                     float * x_modifier,
                     float * x_write_pos,
                     float * y_modifier,
                     float * y_write_pos
                    );

void process_command_list(
                          float * modifiers,
                          Command_Packet ** command_list,
                          Command_Packet ** last,
                          int current_level
                         );

// ### End prototypes.

void setup_globals(void)
{
    printf("Setting up global variables.\n");
    global_variables[gravity] = GRAVITY;
    global_variables[speed] = 0.05f;
    global_variables[is_jumping] = 0.0f;
}

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
    process_keys(window);
}

bool check(GLuint key) {
    return keymap[key];
}

bool max_actions(GLuint key, uint32_t max) {
    if (num_actions[key] >= max) {
        return false;
    }
    num_actions[key]++;
    return true;
}


void create_command_packet(
                           Command_Input * input,
                           Command_Packet ** command_list,
                           Command_Packet ** last
                          )
{
    /* Allocate memory for populate and append a Command_Packet to the supplied
     * list.
     */

    // Allocate memory for new_command.
    Command_Packet * new_command = malloc(sizeof(Command_Packet));

    // Populate Command_Packet.
    new_command->variable = input->variable;
    new_command->value = input->value;
    new_command->mod_function = input->mod_function;
    new_command->lifetime = input->lifetime;
    new_command->next = NULL;
    new_command->sub_commands = NULL;
    new_command->last_sub = NULL;
    new_command->type = input->type;

    // Copy over data to permanent malloc if it's present.
    if (input->data != NULL) {
        // Allocate memory.
        Action_Logic_Data * data_pointer = malloc(sizeof(Action_Logic_Data));
        Action_Logic_Data * input_data = (Action_Logic_Data * )input->data;

        // Populate data pointer.
        data_pointer->logic_function = input_data->logic_function;
        data_pointer->comparison_value = input_data->comparison_value;
        data_pointer->comparison_type = input_data->comparison_type;
        data_pointer->regular_function = input_data->regular_function;
        data_pointer->replacement_function = input_data->replacement_function;
        data_pointer->replacement_value = input_data->replacement_value;

        // Set data to populated data pointer.
        new_command->data = data_pointer;

    } else {
        new_command->data = NULL;
    }

    if (*command_list == NULL) { // Begin new command_list.
        *command_list = new_command;
        *last = new_command;
    } else { // Append to end of current list.
        // Append to last element.
        (*last)->next = new_command;
        // Move last pointer;
        *last = new_command;
    }
}

void submit_command(
                    Command_Input * inputs,
                    size_t size,
                    struct component * component
                   )
{
    /* Submit Command Packet based on input values. */

    // Create first Command_Packet.
    create_command_packet(&inputs[0],
                          &component->command_list,
                          &component->last_command);

    // Append any other packages to the sub_commands head next pointer.
    for (size_t i=1; i<size; i++) {
        create_command_packet(&inputs[i],
                              &component->last_command->sub_commands,
                              &component->last_command->last_sub);
    }
}


float action_add_value(float input, float value, void * data)
{
    UNUSED(data);
    return input+value;
}

float action_set_value(float input, float value, void * data)
{
    UNUSED(data);
    return input=value;
}

float action_logic_wrapper(float input, float value, void * input_data)
{
    // Unpack the data.
    Action_Logic_Data * data = (Action_Logic_Data * )input_data;

    // Compute the next return value from the regular_function.
    float next_value = data->regular_function(input, value, NULL);

    // Compare next value with comparison value through the provided logical
    // function.
    bool next_not_valid = data->logic_function(next_value,
                                               data->comparison_type,
                                               data->comparison_value);

    // If next_value is invalid, replace next_value with the computation of the
    // provided replacement_function.
    if (next_not_valid) {
        float new_value = data->replacement_value;
        next_value = data->replacement_function(input, new_value, NULL);
    }

    // Return the next_value.
    return next_value;
}

float action_print_value(float input, float value, void * input_data)
{
    char * prefix = (char * )input_data;
    printf("%s %f\n", prefix, input);
    return input;
}

void process_keys(GLFWwindow * window)
{

    if (check(GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    // Use controlled components modifiers.
    float * modifiers = controlled_component->modifiers;

    float * x_modifier = &modifiers[X];
    float * y_modifier = &modifiers[Y];

    // Reset modifiers for controlled component.
    *x_modifier = 0;
    *y_modifier = 0;

    float * x_write_pos = &controlled_component->transformation[0][3];
    float * y_write_pos = &controlled_component->transformation[1][3];

    float * current_speed = &global_variables[speed];
    float * current_gravity = &global_variables[gravity];

    float * jump_flag = &global_variables[is_jumping];

    // Modify controlled_component with tab.
    if (check(GLFW_KEY_TAB) && max_actions(GLFW_KEY_TAB, 1)) {
        // Cycle through available components. If NULL return to head.
        controlled_component = controlled_component->next;
        if (controlled_component == NULL) {
            controlled_component = components;
        }
    }

    // Modify positions along x-axis.
    if (check(GLFW_KEY_LEFT)) {
        Command_Input inputs[] = {
           {x_modifier, -(*current_speed), action_add_value, NULL, 1, PASSTHROUGH},
        };
        submit_command(inputs, SIZE(inputs), controlled_component);
    }
    if (check(GLFW_KEY_RIGHT)) {
        Command_Input inputs[] = {
           {x_modifier, +(*current_speed), action_add_value, NULL, 1, PASSTHROUGH},
        };
        submit_command(inputs, SIZE(inputs), controlled_component);
    }

    // Modify positions along y-axis.
    if (check(GLFW_KEY_SPACE) && !(*jump_flag)) {
        Command_Input inputs[] = {
           {jump_flag, 1.0f, action_set_value, NULL, 1, PASSTHROUGH},
           {current_gravity, 0.0f, action_set_value, NULL, 1, PASSTHROUGH},
           {y_modifier, 0.03f, action_add_value, NULL, 20, BLOCKING},
           {current_gravity, GRAVITY, action_set_value, NULL, 1, PASSTHROUGH},
        };
        submit_command(inputs, SIZE(inputs), controlled_component);
    }

    // Process command list for each component.
    struct component * comp_pointer = components;
    while (comp_pointer != NULL) {

        // Process command list.
        // Important that this is done before any global variables like
        // current_gravity is used, since the commands might alter thees values.
        process_command_list(comp_pointer->modifiers,
                             &comp_pointer->command_list,
                             &comp_pointer->last_command, 0);

        // Advance pointer.
        comp_pointer= comp_pointer->next;
    }

    // Add gravity to controlled object.
    Command_Input grav_inputs[] = {
       {y_modifier, -(*current_gravity), action_add_value, NULL, 1, PASSTHROUGH},
    };
    submit_command(grav_inputs, SIZE(grav_inputs), controlled_component);

    // Apply modifications to all components.
    comp_pointer = components;
    while (comp_pointer != NULL) {

        float * modifiers = comp_pointer->modifiers;
        float * mat_transform = &comp_pointer->transformation[0][0];

        float * x_write_pos = &mat_transform[0*4+3];
        float * y_write_pos = &mat_transform[1*4+3];

        float * x_modifier = &modifiers[X];
        float * y_modifier = &modifiers[Y];

        // Check collisions with the window.
        collision_check(mat_transform,
                        x_modifier,
                        x_write_pos,
                        y_modifier,
                        y_write_pos);

        // Write any modified data to the transformation matrix.
        *x_write_pos += *x_modifier;
        *y_write_pos += *y_modifier;

        // Don't forget to reset the modifiers, otherwise the previous
        // modifications will stick if there is a component change.
        *x_modifier = 0.0f;
        *y_modifier = 0.0f;

        // Iterate to next component.
        comp_pointer = comp_pointer->next;
    }
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

    float * bounds = controlled_component->vao->bounds;

    // Get height and widht of bounding box.
    float width = bounds[1*3+0] - bounds[0*3+0];  // 1'st x - 0'th x.
    float height = bounds[0*3+1] - bounds[2*3+1]; // 0'th y - 2'nd y.
    float depth = bounds[4*3+3] - bounds[0*3+3];  // 4'th z - 0'th z.

    // Correctly handle scaling boxes.
    float x_scale = transformation_matrix[0]; // First diagonal pos.
    float y_scale = transformation_matrix[1*3+2]; // Second diagonal pos.
    float z_scale = transformation_matrix[2*3+3]; // Third diagonal pos.

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
    } else if (neg_bound_y_val < y_neg_border) {
        *y_modifier = 0;
        *y_write_pos = y_neg_border + half_height;

        // Reset is jumping flag when hitting floor.
        float * jump_flag = &global_variables[is_jumping];
        Command_Input set_not_jumping[] = {
           {jump_flag, 0.0f, action_set_value, NULL, 1, PASSTHROUGH},
        };
        if (*jump_flag != 0.0f) {
            submit_command(set_not_jumping, SIZE(set_not_jumping), controlled_component);
        }
    }
}

void free_command(Command_Packet * command) {
    // Function used to free command package.

    // Release command data if there is any.
    if (command->data != NULL) {
        free(command->data);
    }
    // Free the command itself.
    free(command);
}

void process_command_list(
                          float * modifiers,
                          Command_Packet ** command_list,
                          Command_Packet ** last,
                          int current_level
                         )
{

    // Dereference to the struct and take pointer to avoid overwriting the
    // anchor when assigning new current.
    Command_Packet * current_pointer = &**command_list;

    // Set up utility pointers.
    Command_Packet * prev = NULL;
    Command_Packet * temp = NULL;

    // Pointers for saving values when de-linking node.
    Command_Packet * temp_next = NULL;
    Command_Packet * temp_last_sub = NULL;

    // Iterate over the command list.
    while(current_pointer != NULL) {

        // Save the current pointer to temp.
        temp = current_pointer;

        // Extract some useful values.
        float * value_to_modify = current_pointer->variable;
        float current_value = *value_to_modify;
        float input_value = current_pointer->value;
        event_action_type action_type = current_pointer->type;
        int has_sub_commands = current_pointer->sub_commands != NULL;
        void * data = current_pointer->data;

        // Apply the modifications in the package.
        *value_to_modify = current_pointer->mod_function(current_value,
                                                         input_value,
                                                         data);

        // Process any sub-commands if type is non-blocking.
        if (has_sub_commands && (action_type != BLOCKING)) {
            process_command_list(modifiers,
                                 &current_pointer->sub_commands,
                                 &current_pointer->last_sub,
                                 current_level+1);
        }

        // Decrease lifetime.
        current_pointer->lifetime -= 1;

        // Three types of unlink scenarios:
        //
        //      1.) [..]->p->c->n->[..]
        //      2.) c->n->[..]
        //      3.) [..]->p->c->N
        //
        //  Legend:
        //      p - prev_pointer.
        //      c - current_pointer.
        //      n - current_pointer->next.
        //      N - NULL.
        //
        //  Given a c that has lifetime == 0, there are three unlinking
        //  scenarios that are possible:
        //
        //      1, The c node is in the middle of the list.
        //      2, The c node is at the beginning of the list.
        //      3, The c node is at the end of the list.
        //          3.1.) End of list with predecessor.
        //          3.2.) Only element in list.
        //
        //  Since the action-list can be nested, the above scenarios exists in
        //  two variants, one where the current node has sub-nodes, and one
        //  where it has none.
        //
        //      y/n the c node has sub-elements.
        //
        //  Detection:
        //      1.) current node has a next node and the prev_pointer is not
        //          NULL.
        //      2.) current->next is not NULL, but prev_pointer == NULL.
        //      3.) current->next is NULL.
        //          3.1.) prev_pointer is not NULL.
        //          3.2.) prev_pointer is NULL.
        //      4.) sub_commands is not NULL.
        //
        //  Actions for unlinking:
        //      1.) Point prev_pointer->next to current_pointer->next.
        //          prev_pointer is still the previous pointer. Advance the
        //          current pointer to the new prev_pointer->next.
        //      2.) Re-write the anchor for the supplied command_list to point
        //          to the current_pointer->next.
        //      3.)
        //          3.1) prev_pointer->next = NULL, move global last pointer.
        //          3.2) Assign command_list = NULL, and set last = NULL.
        //
        //     y/n: The presence of sub-commands does not change the unlinking
        //          process, the main difference is that the sub-elements
        //          should be 'bubbled-up' to the position of their parent in
        //          the main list instead of getting rid of that spot in the
        //          topmost-list.
        //

        if (current_pointer->lifetime == 0) {

            // Update the sub-command variable, the processing of sub-commands
            // might have killed off all the previously defined sub-commands.
            has_sub_commands = current_pointer->sub_commands != NULL;

            if (has_sub_commands) {

                // The sub-command list should not be let to "unfold" if the
                // sub-command list are the only commands left in the global
                // command list. Still want new commands to be run in parallell
                // with all stacks of nested sub-commands. In short, don't let
                // a sub-command list become the new global command list.

                // Save the current next pointer.
                temp_next = temp->next;
                // Save the current sub-commands last pointer.
                temp_last_sub = temp->last_sub;

                // current_pointer = the first sub-command for the dead node.
                current_pointer = current_pointer->sub_commands;
                // Move any other sub commands from the next pointer to the
                // sub_element pointer of the moved node.
                current_pointer->sub_commands = current_pointer->next;

                // Restore old pointers gathered from temp.
                current_pointer->next = temp_next;
                current_pointer->last_sub = temp_last_sub;

                // Check where we are in the list.
                if (prev == NULL) { // First element of the list, re-write anchor and last.
                    *command_list = current_pointer;
                    *last = current_pointer;
                } else { // Not the first or last element, re-link previous to us.
                    prev->next = current_pointer;
                }

                // Check if we are the child to the last element, if so, update
                // the last pointer.
                if (temp->next == NULL) {
                    *last = current_pointer;
                }

                // Advance the previous pointer to the new sub-element. The
                // element has already been processed by the now dead parent
                // element. Don't want to process it twice.
                prev = current_pointer;

                // Advance the current pointer to the next pointer.
                current_pointer = current_pointer->next;

                // Free the dead node.
                free_command(temp);

                // Continue with the iteration.
                continue;

            }  else {  // No sub-elements.

                // De-link the dead node and de-allocate it.

                if (prev == NULL) { // First element.

                    // Set the global anchor and last to the current->next
                    // element. Advance the current pointer and free the
                    // current element stored in temp and continue execution.

                    *command_list = current_pointer->next;
                    *last = current_pointer->next;
                    current_pointer = current_pointer->next;
                    free_command(temp);

                    // Continue iteration.
                    continue;

                } else { // Middle or last in list.

                    if (current_pointer->next == NULL) { // Last element in list.

                        // Re-link previous to NULL and move the global last
                        // pointer.
                        prev->next = NULL;
                        *last = prev;

                    } else { // Middle of list.
                        // Link around the current node.
                        prev->next = current_pointer->next;
                    }

                    // Set the previous node as the current node. Will advance
                    // to the currently assigned next node or NULL further down
                    // in the code.
                    current_pointer = prev;

                }

                // Free the dead current element.
                free_command(temp);
            }
        }

        // Check if this is an iteration in a sub level that is not 0. If we
        // are in a sub-level and declared as blocking, abort all execution of
        // the current evaluation by returning.
        //
        // The point of checking if we are in the main list or not is that
        // blocking commands in the main list should only block the execution
        // of its own sub-commands, not the rest of the actions in the main
        // list. For a blocking sub-command the execution should stop since we
        // want that node to be consumed before any of the nodes below become
        // active.

        if ((action_type == BLOCKING) && (current_level != 0)) {
            return;
        }

        // Advance pointer through the list.
        prev = current_pointer;
        current_pointer = current_pointer->next;
    }
}
