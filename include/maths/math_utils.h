#ifndef MATH_UTILS_H
#define MATH_UTILS_H

// Declare matrix type.
typedef float m4[4][4];

extern m4 m4_identity;

// Copy contents from a m4 to another m4.
void m4_copy(m4 destination, m4 source);

#endif
