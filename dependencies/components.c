#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "globals/globals.h"
#include "events/events.h"
#include "components/components.h"

void update_size(struct component * component);
void set_collision_function(struct component * component, collision_function function);

struct component * create_component(
                                    const char * id,
                                    VAO * vao,
                                    struct component * next
                                   )
    /* Create component on heap and set default values for internal struct
     * variables.
     */
{
    size_t id_len = strlen(id);
    size_t struct_size = sizeof(struct component);
    size_t string_size = (id_len+1)*sizeof(char);
    struct component * return_component = malloc(struct_size);
    if (!return_component) {
        fprintf(stderr, "Could not allocate enough memory for component %s\n", id);
        exit(1);
    }
    // Allocate memory for id.
    return_component->id = malloc(string_size+1);
    if (!return_component->id) {
        fprintf(stderr, "Could not allocate enough memory for id in component %s\n", id);
        exit(1);
    }
    // Copy data from id string to id in struct.
    strncpy(return_component->id, id, id_len);
    // Null terminate the id.
    return_component->id[id_len] = '\0';
    // Populate defaults.
    return_component->vao = vao;
    return_component->command_list = NULL;
    return_component->last_command = NULL;
    return_component->next = next;
    return_component->modifiers[0] = 0;
    return_component->modifiers[1] = 0;
    return_component->modifiers[2] = 0;
    return_component->flags = 0;
    return_component->uniform_data = &standard_uniforms;
    return_component->uniform_size = standard_size;
    // Set transformation matrix to identity matrix.
    m4_copy(return_component->transformation, m4_identity);
    // Set current sizes.
    update_size(return_component);
    // Set standard collision function.
    set_collision_function(return_component, collision_keep_outside_border);

    return return_component;
}

void free_component(struct component * component)
    /* Function for freeing component struct. */
{
    // Iterate over all commands and free them.
    struct Command_Packet * command_pointer = component->command_list;
    struct Command_Packet * temp = NULL;
    while(command_pointer != NULL) {
        temp = command_pointer;
        command_pointer = command_pointer->next;
        free_command(temp);
    }
    free(component->id);
    free(component);
}

uint64_t get_base_mask(enum flag_type flag)
    /* Get the basic mask with a bit set to one at the flag position. */
{
    uint64_t mask = 1;
    return mask <<= flag;
}

void set_flag(struct component * component, enum flag_type flag)
    /* Set the flag at bit "flag" to 1. */
{
    // Get standard mask.
    uint64_t mask = get_base_mask(flag);
    // Or the flag bit to make sure it is 1.
    component->flags |= mask;
}

void wrapper_set_flag(union submit_type * type)
    /* Wrapper metod for set_flag to enable easier internal execution, not ment
     * for manual use. */
{
    struct s_flag * pointer = (struct s_flag * )type;
    set_flag(pointer->component, pointer->flag);
}

void unset_flag(struct component * component, enum flag_type flag)
    /* Set the flag at bit "flag" to 0. */
{
    // Get base mask.
    uint64_t mask = get_base_mask(flag);
    // Invert the mask to one zero and rest ones.
    mask = ~mask;
    // And the mask to make sure that the flag bit is 0.
    component->flags &= mask;
}

void wrapper_unset_flag(union submit_type * type)
    /* Wrapper metod for unset_flag to enable easier internal execution, not ment
     * for manual use. */
{
    struct s_flag * pointer = (struct s_flag * )type;
    unset_flag(pointer->component, pointer->flag);
}

void toggle_flag(struct component * component, enum flag_type flag)
    /* Toggle the value of the flag with XOR. */
{
    uint64_t mask = get_base_mask(flag);
    // XOR the current flag value with mask to toggle the flag bit.
    component->flags ^= mask;
}

bool flag_is_set(struct component * component, enum flag_type flag)
    /* Return if flag is set or not. */
{
    uint64_t mask = get_base_mask(flag);
    return component->flags & mask;
}

bool flag_is_unset(struct component * component, enum flag_type flag)
{
    return !flag_is_set(component, flag);
}

bool controlled_flag_is_unset(enum flag_type flag)
{
    return flag_is_unset(controlled_component, flag);
}

bool controlled_flag_is_set(enum flag_type flag)
{
    return flag_is_set(controlled_component, flag);
}

float * get_modifier(enum coord coordinate, struct component * component)
    /* Return the appropriate coordinate modifier. */
{
    return &component->modifiers[coordinate];
}

float * get_write_location(enum coord coordinate, struct component * component)
    /* Return the appropriate write location for altering component position
     * depending on coordinate type. */
{
    return &component->transformation[coordinate][3];
}

float get_scale(enum coord dimension, struct component * component)
    /* Return the scaling value for the specified dimension. */
{
    return component->transformation[dimension][dimension];
}

void get_scales(struct component * component, v3 result)
    /* Write all scale variables to result. */
{
    for (enum coord coord = 0; coord<NUM_COORD; coord++) {
        result[coord] = get_scale(coord, component);
    }
}

struct component * get_last_component(enum component_list_type type)
    /* Return last component from specific component list. */
{
    return last_component[type];
}

struct component * get_component(enum component_list_type type)
    /* Return first component from selected list. */
{
    return components[type];
}

void append_component(struct component * component,
                      enum component_list_type type)
    /* Append a component to a specific list. */
{
    if (components[type] == NULL) { // New list.
        components[type] = component;
        last_component[type] = component;
    } else { // Populated, append to last.
        get_last_component(type)->next = component;
        last_component[type] = component;
    }
}

void set_as_controlled(struct component * component)
    /* Method for setting which component is the controlled one. */
{
    struct component * temp = controlled_component;

    controlled_component = component;
    controlled_component->uniform_data = &controlled_uniforms;
    controlled_component->uniform_size = controlled_size;

    if (temp != NULL) { // There was a previous controlled component.
        temp->uniform_data = &standard_uniforms;
        temp->uniform_size = standard_size;
    }
}

void write_modifications_to_position(struct component * component)
    /* Write current modifications for X,Y and Z to transformation matrix.
     * Don't forget to reset modifications afters setting them so the do not
     * accumulate. */
{
    for (unsigned int i=0; i < NUM_COORD; i++) {
        const char * id = component->id;
        float * mod_location = get_modifier(i, component);
        float * write_location = get_write_location(i, component);
//        printf("Writing location before %f addition of mod_location: %f for: %s\n", *write_location, *mod_location, id);
        *write_location += *mod_location;
//        printf("Writing location after %f addition of mod_location: %f for: %s\n", *write_location, *mod_location, id);
        // Reset modification.
        *mod_location = 0.0f;
    }
}

float get_position(enum coord coord, struct component * component)
    /* Return current position for component based on supplied coord. */
{
    return component->transformation[coord][3];
}

void set_modifier(
                  enum coord coord,
                  struct component * component,
                  float modifier
                 )
    /* Function to set modifier of a component. */
{
    component->modifiers[coord] = modifier;
}

void reset_modifier(enum coord coord, struct component * component)
    /* Set modifier of component to 0. */
{
    set_modifier(coord, component, 0.0f);
}

void  get_sizes(struct component * component, v3 result)
    /* Populate result v3 with sizes for all axis. */
{
    v3 * bounds = component->vao->bounds;

    result[X] = bounds[1][X] - bounds[0][X];  // 1'st x - 0'th x.
    result[Y] = bounds[0][Y] - bounds[2][Y]; // 0'th y - 2'nd y.
    result[Z] = bounds[4][Z] - bounds[0][Z];  // 4'th z - 0'th z.
}

float get_size(enum coord coord, struct component * component)
    /* Get size for a specific axis. */
{
    v3 result = {0};
    get_sizes(component, result);
    return result[coord];
}

void update_size(struct component * component)
    /* Update stored size parameters. */
{
    v3 sizes = {0};
    v3 scales = {0};

    get_sizes(component, sizes);
    get_scales(component, scales);

    for (int i=0; i<NUM_COORD; i++) {
        component->half_size[i] = sizes[i] * scales[i] / 2.0f;
    }
}

void scale_component(struct component * component, float x, float y, float z)
    /* Apply scaling to transformation matrix and update size. */
{
    m4_scale(component->transformation, 0.4, 0.3, 0.3);
    update_size(component);
}

void set_collision_function(struct component * component, collision_function function)
{
    component->collision_function = function;
}

void get_current_coordinates(struct component * component, v3 * result)
    /* Get the bound values for a component. Write result to the supplied
     * result vector list. */
{
    v3 * vector_list = component->vao->bounds;
    size_t size = SIZE(component->vao->bounds);

    float max_x = 0;
    float min_x = 0;

    get_minmax(X, &min_x, &max_x, vector_list, size);

    float max_y = 0;
    float min_y = 0;

    get_minmax(Y, &min_y, &max_y, vector_list, size);

    float offset_x = component->half_size[X];
    float offset_y = component->half_size[Y];

    // Get current position to use as offset.
    float current_x = get_position(X, component);
    float current_y = get_position(Y, component);

    // Write values to result list.
    result[MIN][X] = current_x - offset_x;
    result[MAX][X] = current_x + offset_x;

    result[MIN][Y] = current_y - offset_y;
    result[MAX][Y] = current_y + offset_y;
}


void get_corners(struct component * component, corners * return_pointer)
    /* Return arrays of v2 structs representing component corners. */
{
    float current_x = get_position(X, component);
    float current_y = get_position(Y, component);

    float half_width = component->half_size[X];
    float half_height = component->half_size[Y];

    corners corner_template = {
        // Top left.
        {current_x - half_width, current_y + half_height, 0.0f},
        // Top Right.
        {current_x + half_width, current_y + half_height, 0.0f},
        // Lower Right.
        {current_x + half_width, current_y - half_height, 0.0f},
        // Lower Left.
        {current_x - half_width, current_y - half_height, 0.0f},
    };

    int num_corners = 4;

    // Copy template to corners.
    for (int i=0; i<num_corners; i++) {
        for (int j=0; j<NUM_COORD; j++) {
            (*return_pointer)[i][j] = corner_template[i][j];
        }
    }
}

void get_corners_next(
                      enum coord coord,
                      struct component * component,
                      corners * current_corners,
                      corners * return_pointer
                     )
    /* Return arrays of v2 structs representing component corners with current
     * modifiers applied.  */
{
    float mod = *get_modifier(coord, component);

    // Copy current values.
    for (int i=0; i<4; i++) {
        for (int j=0; j<NUM_COORD; j++) {
            (*return_pointer)[i][j] = (*current_corners)[i][j];
        }
        // Add modifier.
        (*return_pointer)[i][coord] += mod;
    }
}
