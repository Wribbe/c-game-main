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
float get_scale(enum coord dimension, struct component * component);
void get_scales(struct component * component, v3 result);

// Methods for adding and retrieving different components.
struct component * get_last_component(enum component_list_type type);
struct component * get_component(enum component_list_type type);
void append_component(struct component * component,
                      enum component_list_type type);
void set_as_controlled(struct component * component);
void write_modifications_to_position(struct component * component);
float get_position(enum coord coord, struct component * component);
void set_modifier(enum coord coord,
                  struct component * component,
                  float modifier);
void reset_modifier(enum coord coord, struct component * component);
float get_size(enum coord coord, struct component * component);
void get_sizes(struct component * component, v3 result);
void scale_component(struct component * component, float x, float y, float z);
void set_collision_function(struct component * component, collision_function function);
void get_current_coordinates(struct component * component, v3 * result);
void get_corners(struct component * component, corners * corners);
void get_corners_next(enum coord coord,
                      struct component * component,
                      corners * current_corners,
                      corners * return_pointer);
void get_corners_next_diag(struct component * component,
                           corners * current_corners,
                           corners * return_pointer);
#endif
