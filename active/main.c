#include <stdio.h>
#include <stdlib.h>

#include "gl3w.h"
#include <GLFW/glfw3.h>

#define M_PI 3.14159265358979323846
#define UNUSED(x) (void)x
#define SIZE(x) sizeof(x)/sizeof(x[0])

static void
key_callback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
    UNUSED(scancode);
    UNUSED(mods);

    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
        }
    }
}

GLfloat vertices_rectangle[] = {
    // First triangle.
    -0.5f,  0.5f, 0.0f,
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
    // Second triangle.
    -0.5f,  0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.5f,  0.5f, 0.0f,
};

const GLchar * map_data = \
"##############################\n"
"#                            #\n"
"#                            #\n"
"#                            ###\n"
"#                               #\n"
"#                               #\n"
"#                            ####\n"
"#                            #\n"
"#                            #\n"
"##############################\n";

void
normalize_data(GLuint width, GLuint height, GLfloat * data, size_t size)
{
    GLfloat aspect_ratio = (float)width/(float)height;
    for (size_t i=0; i<size; i += 3) {
        data[i+1] *= aspect_ratio;
    }
}

struct map_row {
    size_t length;
    GLuint * tiles;
};

struct map {
    size_t num_rows;
    size_t max_width;
    struct map_row ** rows;
};

void
die(const GLchar * message)
{
    fprintf(stderr, "[!]: %s, aborting.\n", message);
    exit(EXIT_FAILURE);
}

void *
malloc_or_die(size_t size, const char * error)
{
    void * mem_pointer = malloc(size);
    if (mem_pointer == NULL) {
        die(error);
    }
    return mem_pointer;
}

void
populate_row(const GLchar * start, const GLchar * end, struct map_row * row)
{
    const GLchar * current = start;
    size_t num_tiles = end-start;
    row->tiles = malloc_or_die(sizeof(GLuint)*num_tiles,
            "Could not allocate memory for row->tiles array");
    GLuint * current_tile = row->tiles;
    char c;
    for (; current < end; current++, current_tile++) {
        c = *current;
        switch (c)
        {
            case '#':
                *current_tile = 1;
                break;;
            default:
                *current_tile = 0;
                break;;
        }
    }
    row->length = num_tiles;
}

struct map *
generate_map()
{
    const GLchar * start = map_data;
    const GLchar * end = map_data;

    struct map * map = malloc_or_die(sizeof(struct map),
            "Could not allocate memory for map struct");
    map->rows = NULL;

    size_t num_rows = 0;
    size_t max_len = 0;

    char c;
    for (;;) {
        c = *end++;
        if (c == '\0') {
            break;
        } else if (c != '\n') {
            continue;
        }
        num_rows++;
        size_t len_row = end-start;
        if (map->rows == NULL) {
            map->rows = malloc(sizeof(struct map_row *)*num_rows);
        } else {
            map->rows = realloc(map->rows, sizeof(struct map_row *)*num_rows);
        }
        if (map->rows == NULL) {
            die("Could not re-allocate memory for map-row array");
        }
        map->rows[num_rows-1] = malloc_or_die(sizeof(struct map_row),
                "Could not allocate memory for new map_row struct");
        populate_row(start, end-1, map->rows[num_rows-1]);
        start = end;
        if (len_row > max_len) {
            max_len = len_row;
        }
    }
    map->num_rows = num_rows;
    map->max_width = max_len;
    return map;
}

void
print_map(struct map * map)
{
    struct map_row * current_row = NULL;
    for (size_t i=0; i<map->num_rows; i++) {
        current_row = map->rows[i];
        for (size_t j=0; j<current_row->length; j++) {
            printf("%d", current_row->tiles[j]);
        }
        printf("\n");
    }
}

void
deallocate_map(struct map * map)
{
    for (size_t i=0; i<map->num_rows; i++) {
        struct map_row * current = map->rows[i];
        if (current->tiles != NULL) {
            free(current->tiles);
        }
        free(current);
    }
    free(map->rows);
    free(map);
}


const GLchar * source_fragment = \
"#version 330 core\n"
"void main() {\n"
"   gl_FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
"}\n";

const GLchar * source_vertex = \
"#version 330 core\n"
"layout (location = 0) in vec3 vPosition;\n"
"\n"
"void main() {\n"
"   gl_Position = vec4(vPosition, 1.0f);\n"
"}\n";

GLint
compile_shader(GLuint id_shader)
{
    glCompileShader(id_shader);
    GLint success = 0;
    glGetShaderiv(id_shader, GL_COMPILE_STATUS, &success);
    return success;
}

void
print_shader_error(GLuint id_shader) {
    size_t size_buffer = 1024;
    char buffer_log[size_buffer];
    glGetShaderInfoLog(id_shader, size_buffer, NULL, buffer_log);
    fprintf(stderr, "Compilation of shader failed: %s\n", buffer_log);
}

void
print_program_error(GLuint id_program) {
    size_t size_buffer = 1024;
    char buffer_log[size_buffer];
    glGetProgramInfoLog(id_program, size_buffer, NULL, buffer_log);
    fprintf(stderr, "Compilation of shader failed: %s\n", buffer_log);
}

GLint
assmeble_program(GLuint id_program, GLuint sh1, GLuint sh2)
{
    glAttachShader(id_program, sh1);
    glAttachShader(id_program, sh2);
    glLinkProgram(id_program);
    GLint success = 0;
    glGetProgramiv(id_program, GL_LINK_STATUS, &success);
    return success;
}

int
main(void)
{
    GLFWwindow * window;

    if (!glfwInit()) {
        fprintf(stderr, "Could not initialize glfw, aborting.\n");
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLuint WIDTH = 640;
    GLuint HEIGHT = 480;

    window = glfwCreateWindow(WIDTH, HEIGHT, "HELLO WORLD", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Could not create window, aborting.\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    /* Make window context current one. */
   glfwMakeContextCurrent(window);

    if (gl3wInit()) {
        fprintf(stderr, "Could not initialize gl3w, aborting.\n");
        return EXIT_FAILURE;
    }

    if (!gl3wIsSupported(3, 3)) {
        fprintf(stderr, "Profile 3.3 not supported, aborting.\n");
        return EXIT_FAILURE;
    }

    printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION),
            glGetString(GL_SHADING_LANGUAGE_VERSION));

    /* Set key callback function for main window. */
    glfwSetKeyCallback(window, key_callback);

    /* Create shaders. */
    GLuint shader_vertex = glCreateShader(GL_VERTEX_SHADER);
    GLuint shader_fragment = glCreateShader(GL_FRAGMENT_SHADER);

    /* Set sources. */
    glShaderSource(shader_vertex, 1, &source_vertex, NULL);
    glShaderSource(shader_fragment, 1, &source_fragment, NULL);

    /* Compile shaders. */
    if (compile_shader(shader_vertex) != GL_TRUE) {
        print_shader_error(shader_vertex);
        return EXIT_FAILURE;
    }

    if (compile_shader(shader_fragment) != GL_TRUE) {
        print_shader_error(shader_fragment);
        return EXIT_FAILURE;
    }

    /*  Create shader program. */
    GLuint program = glCreateProgram();
    if (assmeble_program(program, shader_vertex, shader_fragment) != GL_TRUE) {
        print_program_error(program);
        return EXIT_FAILURE;
    }

    /* Delete shaders. */
    glDeleteShader(shader_vertex);
    glDeleteShader(shader_fragment);

    /* Set up OpenGL buffers. */
    GLuint VBO = 0;
    GLuint VAO = 0;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);

    /* Bind vertex array and buffer. */
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    /* Apply aspect ration do data. */
    normalize_data(WIDTH, HEIGHT, vertices_rectangle, SIZE(vertices_rectangle));

    /* Populate VBO with data. */
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_rectangle)*sizeof(GLfloat),
            vertices_rectangle, GL_STATIC_DRAW);

    /* Set and enable correct vertex attribute for vertex position (0). */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    /* Use program. */
    glUseProgram(program);

    /* Generate map structure. */
    struct map * map = generate_map();
    print_map(map);

    while (!glfwWindowShouldClose(window)) {

        /* Render. */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* Draw. */
        glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices_rectangle)/3);

        /* Swap. */
        glfwSwapBuffers(window);

        /* Poll events. */
        glfwPollEvents();

    }

    deallocate_map(map);

    glfwTerminate();
}


