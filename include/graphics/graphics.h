#ifndef INCLUDE_GRAPHICS
#define INCLUDE_GRAPHICS

#define shader_src(x) "src/glsl/"x
#define SIZE(x) sizeof(x)/sizeof(x[0])

float * points;

GLuint vbo;
GLuint vao;

const char * vertex_shader_source;
const char * fragment_shader_source;

void init_memory();
void create_shader(GLuint * shader, GLuint type, const char * source_filename);
void link_program(GLuint * program, GLuint * shaders, size_t size);

#endif
