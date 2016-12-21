#include <stdlib.h>
#include "globals/globals.h"
#include "components/components.h"
#include "structs.h"
#include "graphics/graphics.h"

float global_variables[num_globals];
float EPSILON = 1e-5;

struct uniform_data controlled_uniforms[controlled_size];
struct uniform_data standard_uniforms[standard_size];

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
    global_variables[GRAVITY] = 0.03f;
    global_variables[SPEED] = 0.05f;
    global_variables[JUMP_VELOCITY] = 0.03f;
    global_variables[glfw_time] = 0.0f;

    // Set initial flag values.
    set_flags(get_component(CONTROLLABLE), GRAVITY_ON);
    set_flags(get_component(NON_CONTROLLABLE), GRAVITY_ON);
    set_flags(get_component(CONTROLLABLE), AIRBORN);

    // Set up uniform collections.
    controlled_uniforms[0] = (struct uniform_data){
                                "transform",
                                uniform_data_transform,
                                UniformMatrix4fv,
                             };
    controlled_uniforms[1] = (struct uniform_data){
                                "time",
                                 uniform_data_time,
                                 Uniform1f
                             };

    standard_uniforms[0] = (struct uniform_data){
                                "transform",
                                uniform_data_transform,
                                UniformMatrix4fv,
                           };

    // Set up initial controllable.
    set_as_controlled(get_component(CONTROLLABLE));

    // Set up timestep.
    global_variables[TIMESTEP] = 0.0f;
}

void global_init(void)
{
    // Set background color.
    glClearColor( 0.55f, 0.55f, 0.55f, 0.0f);


    // Set up initial component values.
    for (unsigned int i = 0; i<NUM_COMPONENT_TYPES; i++) {
        components[i] = NULL;
        last_component[i] = NULL;
    }
    controlled_component = NULL;
}


void set_timestep(float new_value)
{
    //printf("Got value: %f\n", new_value);
    global_variables[TIMESTEP] = new_value;
}

float timestep(void)
{
    return global_variables[TIMESTEP];
}

float gravity(void)
{
    return global_variables[GRAVITY]*timestep();
}

float speed(void)
{
    return global_variables[SPEED]*timestep();
}

float jump_velocity(void)
{
    return global_variables[JUMP_VELOCITY]*timestep();
}

float global_speed(void)
{
    return global_variables[SPEED];
}
