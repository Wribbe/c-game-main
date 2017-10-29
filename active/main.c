#include <math.h>
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

struct light_source {
    GLuint x;
    GLuint y;
    GLuint radius;
};

struct player_data {
    size_t x;
    size_t y;
    GLboolean ready;
    double prev_move;
    struct light_source * light;
};


struct node {
    void * data;
    struct node * next;
};

struct node * light_sources;

struct v2i {
    int x;
    int y;
};

struct v2ui {
    size_t x;
    size_t y;
};


struct player_data * player_data;
struct map * current_map;

enum KEYS {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    RESET,
    NUM_KEYS,
};

int key_mapping[NUM_KEYS];
GLboolean key_active[NUM_KEYS];

void
set_player_position(GLuint col, GLuint row)
{
    player_data->x = col;
    player_data->y = row;
    player_data->light->x = col;
    player_data->light->y = row;
}

GLchar
get_tile(GLuint col, GLuint row)
{
    return current_map->rows[row]->tiles[col];
}

GLboolean
player_collides(GLint x, GLint y) {
    if (get_tile(x, y) == ' ') {
        return GL_FALSE;
    }
    return GL_TRUE;
}

GLuint remove_treasure_at(size_t x, size_t y);
void restore_stage(void);

void
perform_actions(void)
{
    GLint diff_x = 0;
    GLint diff_y = 0;

    GLboolean dir_button_pressed = GL_FALSE;

    if (key_active[UP]) {
        diff_y += 1;
        dir_button_pressed = GL_TRUE;
    }
    if (key_active[DOWN]) {
        diff_y -= 1;
        dir_button_pressed = GL_TRUE;
    }
    if (key_active[LEFT]) {
        diff_x -= 1;
        dir_button_pressed = GL_TRUE;
    }
    if (key_active[RIGHT]) {
        diff_x += 1;
        dir_button_pressed = GL_TRUE;
    }
    if (dir_button_pressed == GL_FALSE) {
        /* No movement buttons pressed, ready for next action. */
        player_data->ready = GL_TRUE;
    }
    if (key_active[RESET]) {
        restore_stage();
    }

    GLfloat min_time_between_moves = 0.1f;
    if (glfwGetTime() - player_data->prev_move > min_time_between_moves) {
        player_data->ready = GL_TRUE;
    }

    if ((diff_x || diff_y) && player_data->ready == GL_TRUE) {

        size_t cx = player_data->x;
        size_t cy = player_data->y;

        size_t nx = (size_t)((int)cx+diff_x);
        size_t ny = (size_t)((int)cy+diff_y);

        GLboolean moved_player = GL_TRUE;

        size_t ux = cx;
        size_t uy = cy;

        if (player_collides(nx, ny) == GL_FALSE) {
            ux = nx;
            uy = ny;
            set_player_position(nx, ny);
        } else if (player_collides(cx, ny) == GL_FALSE) {
            uy = ny;
            set_player_position(cx, ny);
        } else if (player_collides(nx, cy) == GL_FALSE) {
            ux = nx;
            set_player_position(nx, cy);
        } else {
            moved_player = GL_FALSE;
        }

        GLuint value = remove_treasure_at(ux, uy);
        if (value > 0) {
            printf("Got treasure worth: %u.\n", value);
        }

        if (moved_player == GL_TRUE) {
            player_data->ready = GL_FALSE;
            player_data->prev_move = glfwGetTime();
        }
    }
}

static void
key_callback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
    UNUSED(scancode);
    UNUSED(mods);

    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        if (key == key_mapping[UP]) {
            key_active[UP] = GL_TRUE;
        }
        if (key == key_mapping[DOWN]) {
            key_active[DOWN] = GL_TRUE;
        }
        if (key == key_mapping[LEFT]) {
            key_active[LEFT] = GL_TRUE;
        }
        if (key == key_mapping[RIGHT]) {
            key_active[RIGHT] = GL_TRUE;
        }
        if (key == key_mapping[RESET]) {
            key_active[RESET] = GL_TRUE;
        }
    } else if (action == GLFW_RELEASE) {
        if (key == key_mapping[UP]) {
            key_active[UP] = GL_FALSE;
        }
        if (key == key_mapping[DOWN]) {
            key_active[DOWN] = GL_FALSE;
        }
        if (key == key_mapping[LEFT]) {
            key_active[LEFT] = GL_FALSE;
        }
        if (key == key_mapping[RIGHT]) {
            key_active[RIGHT] = GL_FALSE;
        }
        if (key == key_mapping[RESET]) {
            key_active[RESET] = GL_FALSE;
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
"#         tttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt  #\n"
"#                  #                                                                        #\n"
"#                  #                                                                     ####\n"
"#    #l       l#   #                                                                     #\n"
"#                  #                                                                     #\n"
"#         g        #                                                                     #\n"
"#                  #                                                                     #\n"
"#                  #                                                                     #\n"
"#    #l       l#   #                                                                     #\n"
"#                  #                                                                     #\n"
"#                  #                                                                     #\n"
"#                  #                                                                     #\n"
"#                  #                                                                     #\n"
"#                                                                                        #\n"
"#                                                                                        #\n"
"#                                                                                        #\n"
"#                                                    p                                   #\n"
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

const GLchar * source_fragment_player = \
"#version 330 core\n"
"void main() {\n"
"   gl_FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);\n"
"}\n";

const GLchar * source_fragment_guard= \
"#version 330 core\n"
"void main() {\n"
"   gl_FragColor = vec4(0.0f, 0.0f, 1.0f, 1.0f);\n"
"}\n";

const GLchar * source_fragment_treasure= \
"#version 330 core\n"
"void main() {\n"
"   gl_FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);\n"
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
assemble_program(GLuint id_program, const GLchar * const source_vertex,
        const GLchar * const source_fragment)
{
    /* Create shaders. */
    GLuint shader_vertex = glCreateShader(GL_VERTEX_SHADER);
    GLuint shader_fragment = glCreateShader(GL_FRAGMENT_SHADER);

    /* Set sources. */
    glShaderSource(shader_vertex, 1, &source_vertex, NULL);
    glShaderSource(shader_fragment, 1, &source_fragment, NULL);

    /* Compile shaders. */
    if (compile_shader(shader_vertex) != GL_TRUE) {
        print_shader_error(shader_vertex);
        exit(EXIT_FAILURE);
    }

    if (compile_shader(shader_fragment) != GL_TRUE) {
        print_shader_error(shader_fragment);
        exit(EXIT_FAILURE);
    }

    glAttachShader(id_program, shader_vertex);
    glAttachShader(id_program, shader_fragment);
    glLinkProgram(id_program);
    GLint success = 0;
    glGetProgramiv(id_program, GL_LINK_STATUS, &success);

    /* Delete shaders. */
    glDeleteShader(shader_vertex);
    glDeleteShader(shader_fragment);

    return success;
}

void
draw_at_grid_pos(struct map * map, GLuint program, GLuint x, GLuint y)
{
    glUseProgram(program);

    GLfloat step_x = map->tile_width;
    GLfloat step_y = map->tile_height;

    /* Grab uniform location for coords. */
    GLint location_coords = glGetUniformLocation(program, "coords");

    GLfloat coords[] = {-1.0f, -1.0f};

    /* Add 0.5 to offset from center of tile. */
    coords[0] += step_x*((float)x+0.5f);
    coords[1] += step_y*((float)y+0.5f);

    coords[0] += map->offset_x;
    coords[1] += map->offset_y;

    glUniform2f(location_coords, coords[0], -coords[1]);
    glDrawArrays(GL_TRIANGLES, 0, SIZE(vertices_rectangle)/3);
}

void
draw_player(GLint program)
{
    glUseProgram(program);
    draw_at_grid_pos(current_map, program, player_data->x, player_data->y);
}

GLboolean
receives_light_from(GLuint x, GLuint y, struct light_source * light)
{
    GLfloat light_x = (GLfloat)light->x;
    GLfloat light_y = (GLfloat)light->y;

    /* Check light radius. */
    GLfloat diff_x = light_x-(GLfloat)x;
    GLfloat diff_y = light_y-(GLfloat)y;
    GLfloat distance = sqrtf(powf(diff_x, 2)+powf(diff_y, 2));

    if (distance > light->radius) {
        return GL_FALSE;
    }

    /* Check for obstructions. */
    GLfloat temp_x = (GLfloat)x;
    GLfloat temp_y = (GLfloat)y;

    diff_x = fabsf(temp_x - light_x);
    diff_y = fabsf(temp_y - light_y);

    for (;;) {

        GLfloat y_step = 1;
        if (diff_x > 0) {
            y_step = diff_y/diff_x;
        }

        /* Walk towards light source. */
        if (temp_x < light_x) {
            temp_x += 1.0f;
        } else if (temp_x > light_x) {
            temp_x -= 1.0f;
        }
        if (temp_y < light_y) {
            temp_y += y_step;
        } else if (temp_y > light_y) {
            temp_y -= y_step;
        }

        if (temp_y == light_y  && temp_x == light_x) {
            break;
        }

        if (get_tile((GLuint)temp_x, (GLuint)temp_y) == '#') {
            /* Obstruction between tile and source. */
            return GL_FALSE;
        }

        diff_x = fabsf(temp_x - light_x);
        diff_y = fabsf(temp_y - light_y);
    }
    return GL_TRUE;
}


GLboolean
is_visible(GLuint x, GLuint y)
{
    if (receives_light_from(x, y, player_data->light) == GL_TRUE) {
        return GL_TRUE;
    }
    for (size_t i=0; i<current_map->num_rows; i++) {
        struct node * node = &light_sources[i];
        while (node != NULL && node->data != NULL) {
            struct light_source * light = (struct light_source *)node->data;
            if (receives_light_from(x, y, light) == GL_TRUE) {
                return GL_TRUE;
            }
            node = node->next;
        }
    }
    return GL_FALSE;
}

void
add_light_source(GLuint x, GLint y, GLuint radius)
{
    struct node * node_p = &light_sources[y];
    while (node_p->next != NULL) {
        node_p = node_p->next;
    }

    struct light_source * source = malloc_or_die(sizeof(struct light_source),
            "Could not allocate memory for new light struct.");
    source->x = x;
    source->y = y;
    source->radius = radius;
    node_p->data = (void*)source;

    struct node * next = malloc_or_die(sizeof(struct node),
            "Could not allocate memory for next node struct");
    next->data = NULL;
    next->next = NULL;
    node_p->next = next;
}

void
deallocate_lights(void)
{
    struct node * current_node = NULL;
    struct node * next_node = NULL;
    for (size_t i=0; i<current_map->num_rows; i++) {
        current_node = &light_sources[i];
        current_node = current_node->next;
        if (current_node != NULL) {
            next_node = current_node->next;
        }
        while (current_node != NULL) {
            free(current_node->data);
            free(current_node);
            current_node = next_node;
            if (current_node != NULL) {
                next_node = current_node->next;
            }
        }
        current_node = &light_sources[i];
        if (current_node->data) {
            free(current_node->data);
        }
    }
}

struct guard {
    struct v2ui position;
    struct v2i direction;
    GLuint view_radius;
};

#define MAX_GUARDS 10
struct guard guards[MAX_GUARDS] = {0};
size_t num_guards = 0;

void
add_guard(size_t x, size_t y)
{
    struct guard * g = &guards[num_guards++];
    if (num_guards > MAX_GUARDS) {
        die("Trying too spawn to many guards");
    }
    g->position.x = x;
    g->position.y = y;
    g->direction.x = 0;
    g->direction.y = -1;
    g->view_radius = 5;
}

struct treasure {
    struct v2ui position;
    GLuint value;
    struct treasure * next;
};

struct treasure * treasures = NULL;
struct treasure * treasures_last = NULL;

void
add_treasure(size_t x, size_t y, GLuint value)
{
    struct treasure * new = malloc_or_die(sizeof(struct treasure),
            "Could not allocate memory for treasure");
    new->position.x = x;
    new->position.y = y;
    new->value = value;
    new->next = NULL;

    if (treasures == NULL) {
        treasures = new;
        treasures_last = treasures;
    } else {
        treasures_last->next = new;
        treasures_last = new;
    }
}

GLuint
remove_treasure_at(size_t x, size_t y)
{
    struct treasure * t = treasures;
    struct treasure * prev = NULL;
    GLuint value = 0;
    for (; t != NULL; t=t->next) {
        if (t->position.x == x && t->position.y == y) {
            if (prev == NULL) {
                treasures = t->next;
            } else {
                prev->next = t->next;
            }
            value = t->value;
            free(t);
            return value;
        }
        prev = t;
    }
    return value;
}


void
deallocate_treasures(void)
{
    struct treasure * t = treasures;
    struct treasure * next = NULL;
    while (t != NULL) {
        next = t->next;
        free(t);
        t = next;
    }
    treasures = NULL;
    treasures_last = NULL;
}

struct info_tile_clear {
    GLchar c;
    size_t x;
    size_t y;
    struct info_tile_clear * next;
};

struct info_tile_clear * tile_clears = NULL;
struct info_tile_clear * tile_clears_last = NULL;

void
clear_pointer(GLchar * tile_pointer, size_t x, size_t y)
{
    size_t size_struct = sizeof(struct info_tile_clear);
    struct info_tile_clear * new = malloc_or_die(size_struct,
           "Could not allocate memory for struct info_tile_clear");

    new->x = x;
    new->y = y;
    new->c = *tile_pointer;
    new->next = NULL;

    *tile_pointer = ' ';
    if (tile_clears == NULL) {
        tile_clears = new;
        tile_clears_last = new;
    } else {
        tile_clears_last->next = new;
        tile_clears_last = new;
    }
}

void
restore_map(void)
{
    struct info_tile_clear * clears = tile_clears;
    struct info_tile_clear * temp = NULL;
    while (clears != NULL) {
        current_map->rows[clears->y]->tiles[clears->x] = clears->c;
        temp = clears->next;
        free(clears);
        clears = temp;
    }
    tile_clears = NULL;
    tile_clears_last = NULL;
}

void
restore_stage(void)
{
    num_guards = 0;
    deallocate_treasures();
    restore_map();
}

void
draw_map(GLint program)
{
    glUseProgram(program);
    struct map * map = current_map;
    GLchar * tile_pointer = NULL;
    for (size_t y = 0; y<map->num_rows; y++) {
        struct map_row * row = map->rows[y];
        tile_pointer = row->tiles;
        for (size_t x = 0; x<row->length; x++) {
            switch (*tile_pointer) {
                case '#':
                    if (is_visible(x, y) == GL_TRUE) {
                        draw_at_grid_pos(map, program, x, y);
                    }
                    break;
                case 'p':
                    set_player_position(x, y);
                    clear_pointer(tile_pointer, x, y);
                    break;
                case 'l':
                    add_light_source(x, y, 10);
                    clear_pointer(tile_pointer, x, y);
                    break;;
                case 't':
                    add_treasure(x, y, 100);
                    clear_pointer(tile_pointer, x, y);
                    break;;
                case 'g':
                    add_guard(x, y);
                    clear_pointer(tile_pointer, x, y);
                    break;;
            }
            tile_pointer++;
        }
    }
}

void
draw_guards(GLint program)
{
    for (size_t i = 0; i<num_guards; i++) {
        draw_at_grid_pos(current_map, program, guards[i].position.x,
                guards[i].position.y);
    }
}

void
draw_treasure(GLint program)
{
    struct treasure * t = treasures;
    for (; t != NULL; t=t->next) {
        draw_at_grid_pos(current_map, program, t->position.x, t->position.y);
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

    /*  Create shader program for map. */
    GLuint map_program = glCreateProgram();
    if (assemble_program(map_program, source_vertex, source_fragment) != GL_TRUE) {
        print_program_error(map_program);
        return EXIT_FAILURE;
    }

    /*  Create shader program for player. */
    GLuint player_program = glCreateProgram();
    if (assemble_program(player_program, source_vertex, source_fragment_player) != GL_TRUE) {
        print_program_error(player_program);
        return EXIT_FAILURE;
    }

    /*  Create shader program for guard. */
    GLuint guard_program = glCreateProgram();
    if (assemble_program(guard_program, source_vertex, source_fragment_guard) != GL_TRUE) {
        print_program_error(guard_program);
        return EXIT_FAILURE;
    }

    /*  Create shader program for traesure. */
    GLuint treasure_program = glCreateProgram();
    if (assemble_program(treasure_program, source_vertex, source_fragment_treasure) != GL_TRUE) {
        print_program_error(treasure_program);
        return EXIT_FAILURE;
    }

    /* Set up OpenGL buffers. */
    GLuint VBO = 0;
    GLuint VAO = 0;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);

    /* Bind vertex array and buffer. */
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    /* Initialize player data. */
    player_data = &(struct player_data){3,3,GL_TRUE,0.0f,&(struct light_source){3,3,15}};

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

    /* Set up key bindings. */
    key_mapping[UP] = GLFW_KEY_D;
    key_mapping[DOWN] = GLFW_KEY_S;
    key_mapping[LEFT] = GLFW_KEY_F;
    key_mapping[RIGHT] = GLFW_KEY_G;
    key_mapping[RESET] = GLFW_KEY_B;

    /* Set up structure for light sources. */
    struct node light_sources_data[map->num_rows];
    for (size_t i=0; i<map->num_rows; i++) {
        light_sources_data[i].data = NULL;
        light_sources_data[i].next = NULL;
    }
    light_sources = light_sources_data;

    while (!glfwWindowShouldClose(window)) {

        /* Render. */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* Poll events. */
        glfwPollEvents();
        perform_actions();

        /* Draw. */
        draw_map(map_program);
        draw_guards(guard_program);
        draw_treasure(treasure_program);
        draw_player(player_program);

        /* Swap. */
        glfwSwapBuffers(window);

    }

    deallocate_lights();
    deallocate_map(map);
    deallocate_treasures();

    glfwTerminate();
}
