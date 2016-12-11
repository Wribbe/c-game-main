#include <stdlib.h>

#include "maths/math_utils.h"

// Copy source
void m4_copy(m4 destination, m4 source) {
    float * dest_array = (float * )destination[0];
    float * src_array = (float * )source[0];
    for (size_t i = 0; i<4; i++) {
        for (size_t j = 0; j<4; j++) {
            size_t index = i*4+j;
            dest_array[index] = src_array[index];
        }
    }
}

void m4_scale(m4 matrix, float x, float y, float z)
{
    matrix[0][0] *= x;
    matrix[1][1] *= y;
    matrix[2][2] *= z;
}

m4 m4_identity = {
    {1.0f,  0.0f, 0.0f, 0.0f},
    {0.0f,  1.0f, 0.0f, 0.0f},
    {0.0f,  0.0f, 1.0f, 0.0f},
    {0.0f,  0.0f, 0.0f, 1.0f},
};
