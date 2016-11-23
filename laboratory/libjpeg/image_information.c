#include <stdlib.h>
#include <stdio.h>

#include "jpeglib/jpeglib.h"
#include "utils/utils.h"

int main(void) {
    printf("%s\n", texture_src("Dietrich.jpeg"));
}
