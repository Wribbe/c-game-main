#include "globals/globals.h"

float global_variables[num_globals];

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
}
