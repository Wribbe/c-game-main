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
    // Set transformation matrix to identity matrix.
    m4_copy(return_component->transformation, m4_identity);

    return return_component;
}

void free_component(struct component * component)
    /* Function for freeing component struct. */
{
    // Iterate over all commands and free them.
    Command_Packet * command_pointer = component->command_list;
    Command_Packet * temp = NULL;
    while(command_pointer != NULL) {
        temp = command_pointer;
        command_pointer = command_pointer->next;
        free_command(temp);
    }
    free(component->id);
    free(component);
}
