#ifndef INCLUDE_GRAPHICS
#define INCLUDE_GRAPHICS

#define shader_src(x) "src/glsl/"x
#define SIZE(x) sizeof(x)/sizeof(x[0])

#define VERTEX_ENDING ".vert"
#define FRAGMENT_ENDING ".frag"

typedef struct {
    GLuint index;
    GLint size;
    GLsizei stride;
    float * offset;
} Attrib_Pointer_Info;

float * points;

GLuint vbo;
GLuint vao;

const char * vertex_shader_source;
const char * fragment_shader_source;

void init_memory();
void create_shader(GLuint * shader, const char * source_filename);
void link_program(GLuint * program, GLuint * shaders, size_t size);

#endif
