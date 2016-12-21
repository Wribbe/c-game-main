#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdbool.h>

#include "structs.h"
#include "image_loaders/image_loaders.h"

#define SIZE(x) sizeof(x)/sizeof(x[0])
#define UNUSED(x) (void)x

#define DATA_SRC "data/"
#define SHADER_SRC "src/glsl/"
#define TEXTURE_SRC "data/textures/"

#define VERTEX_ENDING ".vert"
#define FRAGMENT_ENDING ".frag"


FILE * open_file(const char * filename, size_t * filesize);
size_t read_file(char * data_buffer, size_t filesize, FILE * file_handle);

Point_Data * load_data(const char * filename);
bool logic_main(float left_side, comparison_type comp, float right_side);

char * data_src(const char * filename);
char * shader_src(const char * filename);
char * texture_src(const char * filename);
#endif
