#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "events.h"

struct component {
    const char * id;
    m4 trasformation;
    VAO * vao;
};

#endif
