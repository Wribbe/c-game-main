#ifndef INCLUDE_GRAPHICS
#define INCLUDE_GRAPHICS

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

void load_to_texture(GLuint * texture, const char * filename);

void * uniform_data_transform(struct component * component);

void * uniform_data_time(struct component * component);

void write_data_to_uniform(struct uniform_data * uniform,
                           GLuint location,
                           void * data);

void draw_components(struct component * component,
                     GLuint program,
                     GLuint texture);

void draw_component(struct component * component,
                    GLuint program,
                    GLuint texture);

VAO * create_vao(Point_Data * point_data, GLuint draw_option, GLuint geometry);

#endif
