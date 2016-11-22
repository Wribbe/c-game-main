float * points;

GLuint vbo;
GLuint vao;

GLuint vertex_shader;
GLuint fragment_shader;
GLuint shader_program;

const char * vertex_shader_source;
const char * fragment_shader_source;

void init_memory();
void setup_shaders();
