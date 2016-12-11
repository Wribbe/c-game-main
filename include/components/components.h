#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "structs.h"

struct component * components;
struct component * controlled_component;
struct component * last_component;

struct component * create_component(
                                    const char * id,
                                    VAO * vao,
                                    struct component * next
                                   );

void free_component(struct component * component);

#endif
