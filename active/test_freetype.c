#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gl3w.h"
#include <GLFW/glfw3.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "linmath.h"

#define M_PI 3.14159265358979323846
#define UNUSED(x) (void)x
#define SIZE(x) sizeof(x)/sizeof(x[0])

GLuint WIDTH = 1024;
GLuint HEIGHT = 576;
size_t BYTE_DEPTH = 4;

vec3 camera_pos = {0,0,-1};

mat4x4 m4_unit = {
    {1.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 1.0f},
};

mat4x4 m4_model = {
    {1.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 1.0f},
};

mat4x4 m4_view = {
    {1.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 1.0f},
};

mat4x4 m4_projection = {
    {1.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 1.0f},
};

mat4x4 m4_mvp = {
    {1.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 1.0f},
};

GLuint CURRENT_THICKNESS = 1;

void
die(const GLchar * message)
{
    fprintf(stderr, "[!]: %s, aborting.\n", message);
    exit(EXIT_FAILURE);
}

enum MODS {
    SHIFT,
    NUM_MODS,
};

struct data {
    size_t size;
    size_t num_elements;
    size_t stride;
    GLfloat * values;
};

struct object {
    size_t size;
    GLuint vertex_array;
    GLuint program;
    struct data data_vertex;
    struct data data_uv;
};

#define MAX_CONCURRENT_OBJECTS 200
struct object a_objects[MAX_CONCURRENT_OBJECTS] = {0};
size_t num_current_objects = 0;

struct state_button {
    GLboolean down;
};

struct v3 {
    float x;
    float y;
    float z;
};

struct state_button activity_mods[NUM_MODS] = {0};

GLboolean updated_buttons_keyboard = GL_FALSE;

GLfloat VIEW_FAR = 0.0f;
GLfloat VIEW_NEAR = 0.0f;
GLfloat VIEW_FOVY = M_PI/2;

static void
key_callback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
    UNUSED(scancode);
    UNUSED(mods);

    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        if (key == GLFW_KEY_LEFT_SHIFT) {
            activity_mods[SHIFT].down = GL_TRUE;
        }
        if (key == GLFW_KEY_W) {
            camera_pos[2] += 1;
            printf("Current camera.z = %f\n", camera_pos[2]);
        }
        if (key == GLFW_KEY_S) {
            camera_pos[2] -= 1;
            printf("Current camera.z = %f\n", camera_pos[2]);
        }
        if (key == GLFW_KEY_Q) {
            VIEW_FAR += 0.01f;
            printf("New VIEW_FAR = %f\n", VIEW_FAR);
        }
        if (key == GLFW_KEY_A) {
            VIEW_FAR -= 0.01f;
            printf("New VIEW_FAR = %f\n", VIEW_FAR);
        }
        if (key == GLFW_KEY_E) {
            m4_model[3][3] += 10.0f;
            printf("New model z: %f\n", m4_model[3][3]);
        }
        if (key == GLFW_KEY_D) {
            m4_model[3][3] -= 10.0f;
            printf("New model z: %f\n", m4_model[3][3]);
        }
        if (key == GLFW_KEY_R) {
            VIEW_FOVY += M_PI/8;
            printf("New FOV: %f\n", VIEW_FOVY);
        }
        if (key == GLFW_KEY_F) {
            VIEW_FOVY -= M_PI/8;
            printf("New FOV: %f\n", VIEW_FOVY);
        }
    } else {
        if (key == GLFW_KEY_LEFT_SHIFT) {
            activity_mods[SHIFT].down = GL_FALSE;
        }
    }

    updated_buttons_keyboard = GL_TRUE;
}

GLubyte * texture_data = NULL;

GLuint mouse_x = 0;
GLuint mouse_y = 0;

struct state_button activity_mouse_buttons[4] = {0};

static void
cursor_position_callback(GLFWwindow * window, double xpos, double ypos)
{
    UNUSED(window);

    if (xpos >= WIDTH) {
        xpos = WIDTH-1;
    } else if (xpos < 0) {
        xpos = 0.0f;
    }

    if (ypos >= HEIGHT) {
        ypos = HEIGHT-1;
    } else if (ypos < 0) {
        ypos = 0.0f;
    }

    mouse_x = (GLuint)xpos;
    mouse_y = HEIGHT - (GLuint)ypos - 1;
}

GLboolean updated_buttons_mouse = GL_FALSE;

static void
mouse_button_callback(GLFWwindow * window, int button, int action, int mods)
{
    UNUSED(window);
    UNUSED(mods);
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            activity_mouse_buttons[0].down = GL_TRUE;
        } else {
            activity_mouse_buttons[0].down = GL_FALSE;
        }
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            activity_mouse_buttons[1].down = GL_TRUE;
        } else {
            activity_mouse_buttons[1].down = GL_FALSE;
        }
    }

    updated_buttons_mouse = GL_TRUE;
}

const GLchar * source_fragment = \
"#version 330 core\n"
"\n"
"in vec2 UV;\n"
"\n"
"uniform sampler2D sampler_texture;\n"
"\n"
"void main() {\n"
"   gl_FragColor = texture(sampler_texture, UV);\n"
"}\n";

const GLchar * source_vertex = \
"#version 330 core\n"
"layout (location = 0) in vec3 vPosition;\n"
"layout (location = 1) in vec2 vUV;\n"
"\n"
"uniform mat4 m4_mvp;\n"
"\n"
"out vec2 UV;\n"
"\n"
"void main() {\n"
"   gl_Position = m4_mvp * vec4(vPosition, 1.0f);\n"
"   UV = vUV;\n"
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

GLuint
offset_texture_data(GLuint x, GLuint y)
{
    GLuint bytes_y = WIDTH * BYTE_DEPTH;
    GLuint offset_y = y * bytes_y;
    GLuint offset_x = x * BYTE_DEPTH;
    return offset_y+offset_x;
}

void
color_pixel(GLuint x, GLuint y, GLubyte r, GLubyte g, GLbyte b)
{
    GLubyte * color_pointer = &texture_data[offset_texture_data(x, y)];
    *color_pointer++ = r;
    *color_pointer++ = g;
    *color_pointer++ = b;
}

void
draw_line(GLuint x1, GLuint y1, GLuint x2, GLuint y2, GLuint thickness)
{
    GLuint len_x = abs(x1-x2);
    GLuint len_y = abs(y1-y2);

    GLfloat x_step = 1.0f;
    GLfloat y_step = 1.0f;

    GLboolean x_is_primary = GL_TRUE;
    if (len_x > len_y) {
        y_step = (float)len_y/(float)len_x;
    } else if (len_y > len_x){
        x_step = (float)len_x/(float)len_y;
        x_is_primary = GL_FALSE;
    }

    if (x1 > x2) {
        x_step *= -1;
    }
    if (y1 > y2) {
        y_step *= -1;
    }

    GLfloat current_x = x1;
    GLfloat current_y = y1;

    for (;;) {
        GLuint ux = (GLuint)current_x;
        GLuint uy = (GLuint)current_y;

        if ((x_is_primary && ux == x2) || (!x_is_primary && uy == y2)) {
            break;
        }

        color_pixel(ux, uy, 0, 0, 0);

        current_x += x_step;
        current_y += y_step;

    }
}

void
draw_rectangle(GLuint x1, GLuint y1, GLuint x2, GLuint y2, GLuint thickness)
{
    draw_line(x1, y1, x2, y1, thickness);
    draw_line(x1, y2, x2, y2, thickness);
    draw_line(x1, y1, x1, y2, thickness);
    draw_line(x2, y1, x2, y2, thickness);
}

#define MAX_STORED_COORDS 20
vec3 stored_coords[MAX_STORED_COORDS];
size_t num_stored_coords = 0;

void
reset_stored_mouse(void)
{
    num_stored_coords = 0;
}

GLboolean
stored_mouse_exists(void)
{
    return num_stored_coords > 0;
}

GLboolean DRAW_AT_POINTER = GL_FALSE;

void
store_mouse(void)
    /* Store x,y,z coordinates of mouse, pick x and y from globals, and use
     * camera.z as z-coordinate for the time being.
     * */
{
    if (num_stored_coords > MAX_STORED_COORDS-1) {
        die("Too many stored mouse coordinates, aborting!\n");
    }
    vec3 * current_coord = &stored_coords[num_stored_coords++];
    (*current_coord)[0] = mouse_x;
    (*current_coord)[1] = mouse_y;
    (*current_coord)[2] = camera_pos[2];
}

struct v3
stored_mouse(size_t index)
{
    if (index > MAX_STORED_COORDS-1) {
        die("Trying to access mouse coordinates outside of max, aborting!\n");
    }
    vec3 * current_vec3 = &stored_coords[index];
    return (struct v3){
        (*current_vec3)[0],
        (*current_vec3)[1],
        (*current_vec3)[2],
    };
}


enum input_states {
    STATE_DRAW_LINE,
    STATE_CREATE_RECT,
    NUM_STATES,
};

void
draw_objects(void)
{
    for (size_t i=0; i<num_current_objects; i++) {

        struct object * obj = &a_objects[i];

        glBindVertexArray(obj->vertex_array);
        //glUseProgram(obj->program);
        glDrawArrays(GL_TRIANGLES, 0, obj->data_vertex.size/3);
        glBindVertexArray(0);
    }
}

void
feed_buffer_and_enable_attrib(struct data * data, GLuint attrib_pointer)
{
    /* Put data into buffer. */
    glBufferData(GL_ARRAY_BUFFER, data->size, data->values, GL_STATIC_DRAW);

    /* Specify and enable attribute pointer. */
    GLsizei stride = 0;
    glVertexAttribPointer(attrib_pointer, data->stride, GL_FLOAT,
            GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(attrib_pointer);
}

struct object *
create_object(void)
{
    if (num_current_objects >= MAX_CONCURRENT_OBJECTS) {
        die("To many concurrent objects, aborting.\n");
    }
    return &a_objects[num_current_objects++];
}

void
_obj_set_data(struct data * data_, GLfloat * data, size_t size, size_t stride)
{
    data_->values = malloc(size);
    if (data_->values == NULL) {
        die("Could not allocate memory for values in data struct.\n");
    }
    data_->size = size;
    data_->stride = stride;
    data_->num_elements = size/sizeof(data[0]);
    memcpy(data_->values, data, size);
}

void
obj_set_vertex(struct object * obj, GLfloat * data, size_t size)
{
    _obj_set_data(&obj->data_vertex, data, size, 3);
}

void
obj_set_uv(struct object * obj, GLfloat * data, size_t size)
{
    _obj_set_data(&obj->data_uv, data, size, 2);
}

void
dealocate_objects(void)
{
    for (size_t i=0; i<num_current_objects; i++) {
        struct object * obj = &a_objects[i];
        if (obj->data_vertex.values != NULL) {
            free(obj->data_vertex.values);
        }
        if (obj->data_uv.values != NULL) {
            free(obj->data_uv.values);
        }
    }
}

void
obj_setup_vertex_array(struct object * obj)
{
    GLuint buffer_vertex = 0;
    GLuint buffer_va = 0;

    glGenVertexArrays(1, &buffer_va);
    glBindVertexArray(buffer_va);
    obj->vertex_array = buffer_va;

    glGenBuffers(1, &buffer_vertex);
    glBindBuffer(GL_ARRAY_BUFFER, buffer_vertex);

    feed_buffer_and_enable_attrib(&obj->data_vertex, 0);

    if (obj->data_uv.values != NULL) {
        GLuint buffer_uv = 0;
        glGenBuffers(1, &buffer_uv);
        glBindBuffer(GL_ARRAY_BUFFER, buffer_uv);
        feed_buffer_and_enable_attrib(&obj->data_uv, 1);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void
create_rectangle(void)
{
    struct v3 c1 = stored_mouse(0);
    struct v3 c2 = stored_mouse(1);

    GLfloat c1x = -1.0f + 2.0f*(c1.x/(float)WIDTH);
    GLfloat c2x = -1.0f + 2.0f*(c2.x/(float)WIDTH);

    GLfloat c1y = -1.0f + 2.0f*(c1.y/(float)HEIGHT);
    GLfloat c2y = -1.0f + 2.0f*(c2.y/(float)HEIGHT);

    GLfloat c1z = -1.0f + 2.0f*(c1.z/VIEW_FAR);
    GLfloat c2z = -1.0f + 2.0f*(c2.z/VIEW_FAR);

    c1z = 0.0f;
    c2z = 0.0f;

    GLfloat data_vertice[] = {
        // First triangle.
        c1x, c1y, c1z,
        c1x, c2y, c1z,
        c2x, c2y, c2z,
        // Second triangle
        c1x, c1y, c1z,
        c2x, c2y, c2z,
        c2x, c1y, c2z,
    };

    GLfloat data_uv[] = {
        // First triangle.
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        // Second triangle.
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
    };

    struct object * p_obj = create_object();
    obj_set_vertex(p_obj, data_vertice, sizeof(data_vertice));
    obj_set_uv(p_obj, data_uv, sizeof(data_uv));
    obj_setup_vertex_array(p_obj);

    printf("Input coords: (%f %f %f) (%f %f %f)\n",
            c1.x,
            c1.y,
            c1.z,
            c2.x,
            c2.y,
            c2.z
          );
    printf("Creating rectangle from (%f,%f,%f) to (%f,%f,%f).\n",
            c1x,
            c1y,
            c1z,
            c2x,
            c2y,
            c2z
          );
}

GLboolean input_states[NUM_STATES] = {0};

void
process_input(void)
{
    if (!updated_buttons_keyboard && !updated_buttons_mouse) {
        return;
    }

    if (updated_buttons_mouse) {
        if (activity_mods[SHIFT].down) {
            /* Shift + Mouse1. */
            if (activity_mouse_buttons[0].down) {
                input_states[STATE_DRAW_LINE] = GL_TRUE;
                if (!stored_mouse_exists()) {
                    store_mouse();
                }
            } else if (input_states[STATE_DRAW_LINE]) {
                if (stored_mouse_exists()) {
                    struct v3 stored = stored_mouse(0);
                    draw_line(stored.x, stored.y, mouse_x, mouse_y,
                            CURRENT_THICKNESS);
                    reset_stored_mouse();
                    input_states[STATE_DRAW_LINE] = GL_FALSE;
                }
            }
            /* Shift + Mouse2. */
            if (activity_mouse_buttons[1].down) {
                input_states[STATE_CREATE_RECT] = GL_TRUE;
                if (!stored_mouse_exists()) {
                    store_mouse();
                }
            } else if (input_states[STATE_CREATE_RECT]) {
                if (stored_mouse_exists()) {
                    store_mouse();
                    create_rectangle();
                }
                reset_stored_mouse();
                input_states[STATE_CREATE_RECT] = GL_FALSE;
            }
        } else {
            if (activity_mouse_buttons[0].down) {
                DRAW_AT_POINTER = GL_TRUE;
            } else {
                DRAW_AT_POINTER = GL_FALSE;
            }
            if (activity_mouse_buttons[1].down) {
                if (!stored_mouse_exists()) {
                    store_mouse();
                }
            } else {
                if (stored_mouse_exists()) {
                    struct v3 stored = stored_mouse(0);
                    draw_rectangle(stored.x, stored.y, mouse_x, mouse_y,
                            CURRENT_THICKNESS);
                    reset_stored_mouse();
                }
            }
        }
    }

    updated_buttons_keyboard = GL_FALSE;
    updated_buttons_mouse = GL_FALSE;

}

void
produce_actions(void)
{
    if (DRAW_AT_POINTER) {
        color_pixel(mouse_x, mouse_y, 0, 0, 0);
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
    /* Set mouse position callback function for main window. */
    glfwSetCursorPosCallback(window, cursor_position_callback);
    /* Set mouse button callback function for main window. */
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    /*  Create shader program for map. */
    GLuint basic_program = glCreateProgram();
    if (assemble_program(basic_program, source_vertex, source_fragment) != GL_TRUE) {
        print_program_error(basic_program);
        return EXIT_FAILURE;
    }

    glUseProgram(basic_program);

    /* Create OpenGL texture. */
    GLuint id_texture;
    glGenTextures(1, &id_texture);

    size_t tex_width = WIDTH;
    size_t tex_height = HEIGHT;
    size_t tex_bits = 4;
    texture_data = malloc(sizeof(GLuint)*tex_width*tex_height*tex_bits);
    if (texture_data == NULL) {
        fprintf(stderr, "Failed to allocate memory for texture data, aborting!\n");
        exit(EXIT_FAILURE);
    }
    memset(texture_data, 255-75, tex_width*tex_height*tex_bits);
    memset(texture_data, 255, tex_width*(tex_height/2)*tex_bits);
    /* Bind texture. */
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id_texture);

    /* Set sampler to 0. */
    GLuint location_sampler_texture = glGetUniformLocation(basic_program,
            "sampler_texture");
    glUniform1i(location_sampler_texture, 0);

    /* Give texture data to OpenGL. */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width, tex_height, 0, GL_RGBA,
            GL_UNSIGNED_BYTE, texture_data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glfwSwapInterval(0);

    /* Set up mvp uniform location. */
    GLuint location_m4_mvp = glGetUniformLocation(basic_program, "m4_mvp");

    /* Calculate projection matrix. */
    /* mat4x4_perspective(mat4x4 m, float y_fov, float aspect, float n,
     * float f) */
//    mat4x4_perspective(m4_projection, M_PI/4, (float)WIDTH/(float)HEIGHT,
//            VIEW_NEAR, VIEW_FAR);

    /* Set up camera matrix. */
    /* void mat4x4_look_at(mat4x4 m, vec3 eye, vec3 center, vec3 up) */
    mat4x4_look_at(m4_view,
            camera_pos,
            (vec3){0.0f, 0.0f, 0.0f},
            (vec3){0.0f, 1.0f, 0.0f});

    while (!glfwWindowShouldClose(window)) {

        /* Render. */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        /* Poll events. */
        glfwPollEvents();
        process_input();
        produce_actions();

        /* Re-calculate perspective matrix. */
        mat4x4_perspective(m4_projection, VIEW_FOVY, (float)WIDTH/(float)HEIGHT,
                VIEW_NEAR, VIEW_FAR);

        /* Re-calculate look-at matrix. */
        mat4x4_look_at(m4_view,
                camera_pos,
                (vec3){0.0f, 0.0f, 0.0f},
                (vec3){0.0f, 1.0f, 0.0f});

        /* Re-calculate mvp matrix. */
        mat4x4_mul(m4_mvp, m4_view, m4_model);
        mat4x4_mul(m4_mvp, m4_projection, m4_mvp);

        /* Reload mvp matrix. */
        glUniformMatrix4fv(location_m4_mvp,
                           1,         /* Count. */
                           GL_TRUE,   /* Transpose. */
                           m4_mvp[0]  /* Matrix data. */
                );

        /* Reload texture data. */
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width, tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);

        /* Draw. */
        draw_objects();

        /* Swap. */
        glfwSwapBuffers(window);

    }
    free(texture_data);
    dealocate_objects();
    glfwTerminate();
}
