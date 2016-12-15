#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "components/components.h"
#include "events/events.h"

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
    // Populate defaults.
    return_component->vao = vao;
    return_component->command_list = NULL;
    return_component->last_command = NULL;
    return_component->next = next;
    return_component->modifiers[0] = 0;
    return_component->modifiers[1] = 0;
    return_component->modifiers[2] = 0;
    return_component->flags = 0;
    // Set transformation matrix to identity matrix.
    m4_copy(return_component->transformation, m4_identity);

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
