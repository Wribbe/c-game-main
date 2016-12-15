#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <GLFW/glfw3.h>

#include "globals/globals.h"
#include "events/events.h"
#include "utils/utils.h"
#include "components/components.h"

// ### Prototypes for functions further down in the document.

bool keymap[NUM_KEYS] = {false};
uint32_t num_actions[NUM_KEYS] = {0};

void process_keys(GLFWwindow * window);
void collision_check(
                     struct component * component,
                     float * x_modifier,
                     float * x_write_pos,
                     float * y_modifier,
                     float * y_write_pos
                    );

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

float action_add_value(float input, float * variable, void * data)
{
    UNUSED(data);
    return input+*variable;
}

void wrapper_action_add_value(union submit_type * type)
{
    struct s_float * pointer = (struct s_float * )type;
    *pointer->result = action_add_value(pointer->input, pointer->variable, pointer->data);
    *pointer->signal_result = true;
    *pointer->result_write_pos = pointer->variable;
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



void create_command_packet(
                           struct Command_Packet * input,
                           struct Command_Packet ** command_list,
                           struct Command_Packet ** last
                          )
{
    /* Allocate memory for populate and append a Command_Packet to the supplied
     * list.
     */

    const char * output_tag = "create_command_packet:";

    // Allocate memory for new_command.
    struct Command_Packet * new_command = malloc(sizeof(struct Command_Packet));

    // Set up common attributes.
    new_command->next = NULL;
    new_command->sub_commands = NULL;
    new_command->last_sub = NULL;
    new_command->result = 0.0f;
    new_command->got_result = false;
    new_command->variable_write_pos = NULL;

    int command_type = 0;

    void (*wrapper_function)(union submit_type * type) = NULL;
    void (*remove_function)(union submit_type * type) = NULL;

    int lifetime = 0;
    enum event_action_type action_type = 0;

    enum {
        float_type,
        flag_type,
    };

    // Assume it is flag type.
    struct s_flag * s_flag = (struct s_flag * )&input->type.s_flag;
    if (s_flag->function == set_flag) {

        command_type = flag_type;
        wrapper_function = wrapper_set_flag;
        remove_function = free_s_flag;

    } else if (s_flag->function == unset_flag) {

        command_type = flag_type;
        wrapper_function = wrapper_unset_flag;
        remove_function = free_s_flag;

    }

    if (wrapper_function == NULL) { // Not found anything yet.

        // Assume it is float type.
        struct s_float * s_float = (struct s_float * )&input->type.s_float;

        if (input->type.s_float.function == action_add_value) {

            command_type = float_type;
            wrapper_function = wrapper_action_add_value;
            remove_function = free_s_float;

        }
    }

    if (wrapper_function == NULL || remove_function == NULL) {
        // No match, crash.
        const char * error = "%s Unknown input type, can't determine func methods!\n";
        fprintf(stderr, error, output_tag);
        exit(1);
    }

    if (command_type == flag_type) {

        struct s_flag * flag_input = (struct s_flag * )&input->type.s_flag;
        struct s_flag * flag_new = (struct s_flag * )&new_command->type.s_flag;

        flag_new->function = flag_input->function;
        flag_new->flag = flag_input->flag;
        flag_new->component = flag_input->component;
        lifetime = flag_input->lifetime;
        action_type = flag_input->action_type;

    } else if (command_type == float_type) {

        struct s_float * float_input = (struct s_float* )&input->type.s_float;
        struct s_float * float_new = (struct s_float* )&new_command->type.s_float;

        float_new->function = float_input->function;
        float_new->input = float_input->input;
        float_new->variable = float_input->variable;
        lifetime = float_input->lifetime;
        action_type = float_input->action_type;
        float_new->result = &new_command->result;
        float_new->signal_result = &new_command->got_result;
        float_new->result_write_pos = &new_command->variable_write_pos;

        // Copy over data to permanent malloc if it's present.
        if (float_input->data != NULL) {
            // Allocate memory.
            Action_Logic_Data * data_pointer = malloc(sizeof(Action_Logic_Data));
            Action_Logic_Data * input_data = (Action_Logic_Data * )float_input->data;

            // Populate data pointer.
            data_pointer->logic_function = input_data->logic_function;
            data_pointer->comparison_value = input_data->comparison_value;
            data_pointer->comparison_type = input_data->comparison_type;
            data_pointer->regular_function = input_data->regular_function;
            data_pointer->replacement_function = input_data->replacement_function;
            data_pointer->replacement_value = input_data->replacement_value;

            // Set data to populated data pointer.
            float_new->data = data_pointer;

            } else {
                float_new->data = NULL;
            }
    }

    // Set wrapper and remove function for new command.
    new_command->wrapper_function = wrapper_function;
    new_command->remove = remove_function;

    // Set lifetime of command.
    new_command->lifetime = lifetime;

    // Set action type.
    new_command->action_type = action_type;

    if (*command_list == NULL) { // Begin new command_list.
        *command_list = new_command;
        *last = new_command;
    } else { // Append to end of current list.
        // Append to last element.
        (*last)->next = new_command;
        // Move last pointer;
        *last = (*last)->next;
    }
}

void submit_command(
                    struct Command_Packet * inputs,
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
        struct Command_Packet inputs[] = {
           {.type.s_float = {
                    action_add_value,
                    -(*current_speed),
                    x_modifier,
                    1,
                    NULL,
                    PASSTHROUGH,
                }
           },
        };
        submit_command(inputs, SIZE(inputs), controlled_component);
    }

    if (check(GLFW_KEY_RIGHT)) {
        struct Command_Packet inputs[] = {
           {.type.s_float = {
                    action_add_value,
                    +(*current_speed),
                    x_modifier,
                    1,
                    NULL,
                    PASSTHROUGH,
                }
           },
        };
        submit_command(inputs, SIZE(inputs), controlled_component);
    }

    // Modify positions along y-axis.
    if (check(GLFW_KEY_SPACE) && controlled_flag_is_unset(JUMPING)) {
        set_flag(controlled_component, JUMPING);
        struct Command_Packet inputs[] = {
            {.type.s_flag = {
                    unset_flag, GRAVITY_ON, controlled_component, 1, PASSTHROUGH
                }
            },
            {.type.s_float = {
                    action_add_value, 0.03f, y_modifier, 20, NULL, BLOCKING
                }
            },
            {.type.s_flag = {
                    set_flag, GRAVITY_ON, controlled_component, 1, PASSTHROUGH
                }
            },
        };
        submit_command(inputs, SIZE(inputs), controlled_component);
    }

    // Process command list for each component.
    struct component * comp_pointer = components;
    while (comp_pointer != NULL) {

        // Process command list.
        // Important that this is done before any global variables like
        // current_gravity is used, since the commands might alter thees values.
        process_command_list(comp_pointer->modifiers, &comp_pointer->command_list, &comp_pointer->last_command, 0);

        // Advance pointer.
        comp_pointer= comp_pointer->next;
    }

    if (controlled_flag_is_set(GRAVITY_ON)) {
        // Add gravity to controlled object.
        struct Command_Packet grav_inputs[] = {
           {.type.s_float = {
                    action_add_value,
                    -(*current_gravity),
                    y_modifier,
                    1,
                    NULL,
                    PASSTHROUGH,
                }
           },
        };
        submit_command(grav_inputs, SIZE(grav_inputs), controlled_component);
    }

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
        collision_check(comp_pointer,
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

float get_bound_dimension(enum coord dimension, struct component * component)
    /* Return the correct value of bounds depending on dimension. */
{
    v3 * bounds = &component->vao->bounds[0][0];

    switch(dimension) {
        case X:
            return bounds[1][X] - bounds[0][X];  // 1'st x - 0'th x.
            break;
        case Y:
            return bounds[0][Y] - bounds[2][Y]; // 0'th y - 2'nd y.
            break;
        case Z:
            return bounds[4][Z] - bounds[0][Z];  // 4'th z - 0'th z.
            break;
        default:
            fprintf(stderr, "get_bound_dimension: No such dimension.\n");
            exit(1);
            break;
    }
}

float get_scale(float * transformation_matrix, enum coord dimension)
    /* Return scaling factor for a specific coordinate from the given
     * transformation matrix. */
{
    switch (dimension) {
        case X:
            return transformation_matrix[0]; // First diagonal pos.
        case Y:
            return transformation_matrix[1*3+2]; // Second diagonal pos.
        case Z:
            return transformation_matrix[1*3+2]; // Second diagonal pos.
        default:
            fprintf(stderr, "get_scale: No such dimension, aborting.\n");
            exit(1);
            break;
    }
}

void set_variable(
                  enum coord type,
                  struct component * component,
                  float * var_modifier,
                  float * var_write_pos,
                  float collision_modfifier_value,
                  struct collision_bound_data * bound_data,
                  size_t num_bounds
                 )
    /* Function handling the setting of y modifier and position for supplied
     * y_write_pos. */
{
    float relevant_component_size = 0;
    float relevant_scale = 0;

    float next_var = *var_write_pos + *var_modifier;

    relevant_component_size = get_bound_dimension(type, component);

    //Scale the relevant_component_size.
    relevant_component_size *= get_scale(component->transformation, type);

    // Signed half component size depending on current direction.
    float additional_to_next = 0;
    float half_relevant = relevant_component_size/2.0f;
    if (*var_modifier < 0) { // Going down/left.
        additional_to_next -= half_relevant;
    } else { // Going up/right.
        additional_to_next += half_relevant;
    }


    // Add additional offset to the calculated next_var.
    next_var += additional_to_next;

    // Compare to bounds.
    for(size_t i=0; i<num_bounds; i++) {
        struct collision_bound_data * data = &bound_data[i];
        if(logic_main(next_var, data->compare_type, data->bound)) {
            *var_modifier = 0;
            *var_write_pos = data->value;
            if (data->flag_operation != NULL) {
                data->flag_operation(component, data->flag);
            }
            break;
        }
    }
}

void collision_check(
                     struct component * component,
                     float * x_modifier,
                     float * x_write_pos,
                     float * y_modifier,
                     float * y_write_pos
                    )
{
    /* Check collisions against window border. If collision detected set the
     * position of the object at the edge of the window bounding box.
     */

//    float * bounds = component->vao->bounds;
    float * transformation_matrix = &component->transformation[0][0];

    // Get height and widht of bounding box.
    float width = get_bound_dimension(X, component);
    float height = get_bound_dimension(Y, component);
    float depth = get_bound_dimension(Z, component);

    // Correctly handle scaling boxes.
    float x_scale = transformation_matrix[0]; // First diagonal pos.
    float y_scale = transformation_matrix[1*3+2]; // Second diagonal pos.
    float z_scale = transformation_matrix[2*3+3]; // Third diagonal pos.

    // Scale width and height;
    width *= x_scale;
    height *= y_scale;
    depth *= z_scale;

    float x_pos_border = global_variables[wb_x_pos];
    float x_neg_border = global_variables[wb_x_neg];
    float y_pos_border = global_variables[wb_y_pos];
    float y_neg_border = global_variables[wb_y_neg];

    // Calculate next position for x and y.
    float next_x = *x_write_pos + *x_modifier;
    float next_y = *y_write_pos + *y_modifier;

    float half_width = width/2.0f;
    float half_height = height/2.0f;

    float pos_bound_x_val = next_x + half_width;
    float neg_bound_x_val = next_x - half_width;

    float pos_bound_y_val = next_y + half_height;
    float neg_bound_y_val = next_y - half_height;

    // Check for collisions with window along x-axis.
    struct collision_bound_data x_bounds[] = {
        {1.0f, x_pos_border-half_width, GT, NULL, 0},
        {-1.0f, x_neg_border+half_width, LT, NULL, 0},
    };

    set_variable(X,
                 component,
                 x_modifier,
                 x_write_pos,
                 0,
                 x_bounds,
                 SIZE(x_bounds));

    // Check for collisions with window along y-axis.
    struct collision_bound_data y_bounds[] = {
        {1.0f, y_pos_border-half_height, GT, NULL, 0},
        {-1.0f, y_neg_border+half_height, LT, unset_flag, JUMPING},
    };

    set_variable(Y,
                 component,
                 y_modifier,
                 y_write_pos,
                 0,
                 y_bounds,
                 SIZE(y_bounds));
}

void free_command(struct Command_Packet * command) {
    // Function used to free command package.

    // Release the type first.
    union submit_type * type = &command->type;
    // Use stored remove to free any of types internal resources.
    command->remove(type);

    // Free any sub-commands recursively.
    struct Command_Packet * sub_command_pointer = command->sub_commands;
    struct Command_Packet * sub_temp = NULL;
    while (sub_command_pointer != NULL) {
        sub_temp = sub_command_pointer;
        sub_command_pointer = sub_command_pointer->next;
        free_command(sub_temp);
    }
    // Free the command itself.
    free(command);
}

void process_command_list(
                          float * modifiers,
                          struct Command_Packet ** command_list,
                          struct Command_Packet ** last,
                          int current_level
                         )
{

    // Dereference to the struct and take pointer to avoid overwriting the
    // anchor when assigning new current.
    struct Command_Packet * current_pointer = &**command_list;

    // Set up utility pointers.
    struct Command_Packet * prev = NULL;
    struct Command_Packet * temp = NULL;

    // Pointers for saving values when de-linking node.
    struct Command_Packet * temp_next = NULL;
    struct Command_Packet * temp_last_sub = NULL;

    // Iterate over the command list.
    while(current_pointer != NULL) {

        // Save the current pointer to temp.
        temp = current_pointer;

        // Get command data.
        enum event_action_type action_type = current_pointer->action_type;
        bool has_sub_commands = current_pointer->sub_commands != NULL;

        // Run wrapper function.
        current_pointer->wrapper_function(&current_pointer->type);

        // Did we get any results?
        if (current_pointer->got_result) {
            float * value_to_modify = current_pointer->variable_write_pos;
            *value_to_modify = current_pointer->result;
        }

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

                // Remove pointers to sub-command in temp since free_command
                // removes sub-commands recursively.
                temp->sub_commands = NULL;

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
