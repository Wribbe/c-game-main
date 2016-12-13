#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "structs.h"
#include "globals/globals.h"

struct component * components;
struct component * controlled_component;
struct component * last_component;

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

#endif
