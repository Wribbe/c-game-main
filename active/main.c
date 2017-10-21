#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gl3w.h"
#include <GLFW/glfw3.h>

#define M_PI 3.14159265358979323846
#define UNUSED(x) (void)x
#define SIZE(x) sizeof(x)/sizeof(x[0])

struct map_row {
    size_t length;
    GLchar * tiles;
};

struct map {
    size_t num_rows;
    size_t max_width;
    GLfloat offset_x;
    GLfloat offset_y;
    GLfloat tile_width;
    GLfloat tile_height;
    struct map_row ** rows;
};

struct player_data {
    GLuint row;
    GLuint col;
};

struct player_data * player_data;
struct map * current_map;

enum KEYS {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    NUM_KEYS,
};

int key_mapping[NUM_KEYS];

void
set_player_position(GLuint col, GLuint row)
{
    player_data->row = row;
    player_data->col = col;
}

GLchar
get_tile(GLuint col, GLuint row)
{
    return current_map->rows[row]->tiles[col];
}

GLboolean
player_collides(GLint coordinates[2]) {
    if (get_tile(coordinates[0], coordinates[1]) == ' ') {
        return GL_FALSE;
    }
    return GL_TRUE;
}

static void
key_callback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
    UNUSED(scancode);
    UNUSED(mods);

    if (action == GLFW_PRESS) {
        GLint new_player_coords[2] = {player_data->col,player_data->row};
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        } else if (key == key_mapping[UP]) {
            new_player_coords[1] += 1;
        } else if (key == key_mapping[DOWN]) {
            new_player_coords[1] -= 1;
        } else if (key == key_mapping[LEFT]) {
            new_player_coords[0] -= 1;
        } else if (key == key_mapping[RIGHT]) {
            new_player_coords[0] += 1;
        }
        if (player_collides(new_player_coords) == GL_FALSE) {
            set_player_position(new_player_coords[0], new_player_coords[1]);
        }
    }
}

GLfloat vertices_rectangle[] = {
    // First triangle.
    -1.0f,  1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
    // Second triangle.
    -1.0f,  1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
     1.0f,  1.0f, 0.0f,
};

const GLchar * map_data = \
"##########################################################################################\n"
"#                                                                                        #\n"
"#                                                                                        ###\n"
"#                                                                                           #\n"
"#                                                                                           #\n"
"#                                                                                        ####\n"
"#    #         #                                                                         #\n"
"#                                                                                        #\n"
"#                                                                                        #\n"
"#                                                                                        #\n"
"#                                                                                        #\n"
"#    #         #                                                                         #\n"
"#                                                                                        #\n"
"#                                                                                        #\n"
"#                                                                                        #\n"
"#                                                                                        #\n"
"#                                                                                        #\n"
"#                                                                                        #\n"
"#                                                                                        #\n"
"#                                                                                        #\n"
"#                                                                                        #\n"
"#                                                                                        #\n"
"#                                                                                        #\n"
"#                                                                                        #\n"
"#                                                   p                                    #\n"
"#                                                                                        #\n"
"#                                                                                        #\n"
"#                                                                                        #\n"
"#                                                                                        #\n"
"#                                                                                        #\n"
"#                                                                                        #\n"
"#                                                                                        #\n"
"#                                                                                        #\n"
"#                                                                                        #\n"
"##########################################################################################\n";

void
scale_tiles(struct map * map, GLuint width, GLuint height)
{
    GLfloat pixel_width = 2.0f/(float)width;
    GLfloat pixel_height = 2.0f/(float)height;

    GLfloat tile_fitted_to_height = (float)height/(float)map->num_rows;
    GLfloat tile_fitted_to_widht = (float)width/(float)map->max_width;

    GLfloat tile_size_base = tile_fitted_to_widht;
    if (tile_fitted_to_widht * pixel_width *  map->num_rows > 2.0f) {
        tile_size_base = tile_fitted_to_height;
    }

    GLfloat tile_width = tile_size_base*pixel_width;
    GLfloat tile_height = tile_size_base*pixel_height;

    GLfloat tile_dim[] = {tile_width/2, tile_height/2};

    /* Construct new tile. */
    GLfloat fitted_tile[] = {
        /* First triangle. */
        -tile_dim[0], -tile_dim[1], 0.0f,
        -tile_dim[0],  tile_dim[1], 0.0f,
         tile_dim[0],  tile_dim[1], 0.0f,
        /* Second triangle. */
        -tile_dim[0], -tile_dim[1], 0.0f,
         tile_dim[0],  tile_dim[1], 0.0f,
         tile_dim[0], -tile_dim[1], 0.0f,
    };
    /* Write over old data. */
    memcpy(vertices_rectangle, fitted_tile, sizeof(vertices_rectangle));

    map->offset_x = 0;
    map->offset_y = 0;

    if (tile_fitted_to_widht * map->num_rows * pixel_width > 2.0f) {
        map->offset_x = (2.0f-(tile_width*map->max_width))/2.0f;
    } else {
        map->offset_y = (2.0f-(tile_height*map->num_rows))/2.0f;
    }
    map->tile_width = tile_width;
    map->tile_height = tile_height;
}

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
    size_t num_tiles = end-start;
    row->tiles = malloc_or_die(sizeof(GLchar)*(num_tiles+1),
            "Could not allocate memory for row->tiles array");
    memcpy(row->tiles, start, num_tiles);
    row->tiles[num_tiles] = '\0';
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
        size_t len_row = end-start-1;
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
            printf("%c", current_row->tiles[j]);
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
"uniform vec2 coords;\n"
"\n"
"void main() {\n"
"   gl_Position = vec4(vPosition+vec3(coords, 0.0f), 1.0f);\n"
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

void
draw_at_grid_pos(struct map * map, GLuint program, GLuint row, GLuint col)
{
    GLfloat step_x = map->tile_width;
    GLfloat step_y = map->tile_height;

    /* Grab uniform location for coords. */
    GLint location_coords = glGetUniformLocation(program, "coords");

    GLfloat coords[] = {-1.0f, -1.0f};

    /* Add 0.5 to offset from center of tile. */
    coords[0] += step_x*((float)col+0.5f);
    coords[1] += step_y*((float)row+0.5f);

    coords[0] += map->offset_x;
    coords[1] += map->offset_y;

    glUniform2f(location_coords, coords[0], -coords[1]);
    glDrawArrays(GL_TRIANGLES, 0, SIZE(vertices_rectangle)/3);
}

void
draw_player(GLint program)
{
    draw_at_grid_pos(current_map, program, player_data->row, player_data->col);
}

void
draw_map(GLint program)
{
    struct map * map = current_map;
    GLchar * tile_pointer = NULL;
    for (size_t index_row = 0; index_row<map->num_rows; index_row++) {
        struct map_row * row = map->rows[index_row];
        tile_pointer = row->tiles;
        for (size_t index_col = 0; index_col<row->length; index_col++) {
            switch (*tile_pointer) {
                case '#':
                    draw_at_grid_pos(map, program, index_row, index_col);
                    break;
                case 'p':
                    set_player_position(index_col, index_row);
                    *tile_pointer = ' ';
                    break;
            }
            tile_pointer++;
        }
    }
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

    GLuint WIDTH = 1024;
    GLuint HEIGHT = 576;

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

    /* Initialize player data. */
    player_data = &(struct player_data){3,3};

    /* Generate map structure. */
    struct map * map = generate_map();
    current_map = map;
    print_map(map);

    /* Scale vertex data to fit the map and screen. */
    scale_tiles(map, WIDTH, HEIGHT);

    /* Populate VBO with data. */
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_rectangle)*sizeof(GLfloat),
            vertices_rectangle, GL_STATIC_DRAW);

    /* Set and enable correct vertex attribute for vertex position (0). */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    /* Use program. */
    glUseProgram(program);

    /* Set up key bindings. */
    key_mapping[UP] = GLFW_KEY_D;
    key_mapping[DOWN] = GLFW_KEY_S;
    key_mapping[LEFT] = GLFW_KEY_F;
    key_mapping[RIGHT] = GLFW_KEY_G;

    while (!glfwWindowShouldClose(window)) {

        /* Render. */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* Poll events. */
        glfwPollEvents();

        /* Draw. */
        draw_map(program);
        draw_player(program);

        /* Swap. */
        glfwSwapBuffers(window);

    }

    deallocate_map(map);

    glfwTerminate();
}
