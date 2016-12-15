#include <stdlib.h>
#include "structs.h"

void free_s_flag(union submit_type * type)
    /* Nothing additionally allocated. */
{
}

void free_s_float(union submit_type * type)
    /* Data might be allocated, check that first. */
{
    struct s_float * s_float = (struct s_float * )type;
    if (s_float->data != NULL) {
        free(s_float->data);
    }
}
