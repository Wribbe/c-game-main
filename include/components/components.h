#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "structs.h"
#include "globals/globals.h"

struct component * create_component(
                                    const char * id,
                                    VAO * vao,
                                    struct component * next
                                   );

void free_component(struct component * component);

// Set flag bits.
void set_flag(struct component * component, enum flag_type flag);
void unset_flag(struct component * component, enum flag_type flag);
void toggle_flag(struct component * component, enum flag_type flag);

// Check flag bits.
bool flag_is_set(struct component * component, enum flag_type flag);
bool flag_is_unset(struct component * component, enum flag_type flag);
bool controlled_flag_is_unset(enum flag_type flag);
bool controlled_flag_is_set(enum flag_type flag);

// Wrapper functions for automatic command execution.
void wrapper_set_flag(union submit_type * type);
void wrapper_unset_flag(union submit_type * type);

// Functions handling altering component coordinate modifiers and position
// values.
float * get_modifier(enum coord coordinate, struct component * component);
float * get_write_location(enum coord coordinate, struct component * component);
float get_dimension_scale(enum coord dimension, struct component * component);

// Methods for adding and retrieving different components.
struct component * get_last_component(enum component_list_type type);
struct component * get_component(enum component_list_type type);
void append_component(struct component * component,
                      enum component_list_type type);
void set_as_controlled(struct component * component);
#endif
