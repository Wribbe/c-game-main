#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include "structs.h"
#include "image_loaders/image_loaders.h"

#define SIZE(x) sizeof(x)/sizeof(x[0])

#define data_src(x) "data/"x
#define shader_src(x) "src/glsl/"x
#define texture_src(x) "data/textures/"x

#define VERTEX_ENDING ".vert"
#define FRAGMENT_ENDING ".frag"

FILE * open_file(const char * filename, size_t * filesize);
size_t read_file(char * data_buffer, size_t filesize, FILE * file_handle);

void load_data(Point_Data * info, float * buffer, const char *  filename);

#endif
