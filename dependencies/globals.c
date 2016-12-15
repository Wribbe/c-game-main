#include <stdlib.h>
#include "globals/globals.h"
#include "components/components.h"

float global_variables[num_globals];

void set_flags(struct component * component, enum flag_type flag)
    /* Set flag for the whole list of component. */
{
    // Set initial flag values for all entries.
    struct component * component_pointer = component;
    while (component_pointer != NULL) {
        set_flag(component_pointer, flag);
        // Advance pointer.
        component_pointer = component_pointer->next;
    }
}

void setup_globals(void)
{
    printf("Setting up global variables.\n");
    global_variables[gravity] = GRAVITY;
    global_variables[speed] = 0.05f;
    global_variables[is_jumping] = 0.0f;
    global_variables[glfw_time] = 0.0f;
    global_variables[wb_x_pos] =  1.0f;
    global_variables[wb_x_neg] = -1.0f;
    global_variables[wb_y_pos] =  1.0f;
    global_variables[wb_y_neg] = -1.0f;

    // Set initial flag values.
    set_flags(get_component(CONTROLLABLE), GRAVITY_ON);
    set_flags(get_component(NON_CONTROLLABLE), GRAVITY_ON);

    // Set up initial component values.
    struct components * components = {0};
    struct components * last_components = {0};
    struct components * controlled_component = NULL;
}
