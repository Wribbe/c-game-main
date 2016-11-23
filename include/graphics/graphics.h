#ifndef INCLUDE_GRAPHICS
#define INCLUDE_GRAPHICS

#define shader_src(x) "src/glsl/"x
#define SIZE(x) sizeof(x)/sizeof(x[0])

#define VERTEX_ENDING ".vert"
#define FRAGMENT_ENDING ".frag"

#include "utils/utils.h"
#include "structs.h"

void create_shader(GLuint * shader, const char * source_filename);
void link_program(GLuint * program, GLuint * shaders, size_t size);

void gen_buffers(GLuint num_buffers,
                 VBO * vbo,
                 Point_Data * data,
                 GLuint draw_type);

void gen_vertex_arrays(GLuint num_buffers,
                       VAO * vao,
                       VBO * vbo_binds[],
                       size_t num_vbos,
                       Attrib_Pointer_Info * enable_arrayp,
                       size_t num_arrayp);
#endif
