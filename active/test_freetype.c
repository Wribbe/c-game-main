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

vec3 camera_pos = {0,0,3};

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
            camera_pos[2] += 1.5f;
        }
        if (key == GLFW_KEY_S) {
            camera_pos[2] -= 1.5f;
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
                if (!stored_mouse_exists()) {
                    store_mouse();
                }
            } else {
                if (stored_mouse_exists()) {
                    struct v3 stored = stored_mouse(0);
                    draw_line(stored.x, stored.y, mouse_x, mouse_y,
                            CURRENT_THICKNESS);
                    reset_stored_mouse();
                }
            }
            /* Shift + Mouse2. */
            if (activity_mouse_buttons[1].down) {
                if (!stored_mouse_exists()) {
                    store_mouse();
                }
                printf("Pressed SHIFT + MOUSE2!\n");
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

    /* Set up OpenGL buffers. */
    GLuint VBO = 0;
    GLuint VAO = 0;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);

    /* Bind vertex array and buffer. */
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    /* Populate VBO with data. */
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_rectangle)*sizeof(GLfloat),
            vertices_rectangle, GL_STATIC_DRAW);

    /* Set and enable correct vertex attribute for vertex position (0). */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    /* Set up UV buffer. */
    GLuint buffer_uv = 0;
    glGenBuffers(1, &buffer_uv);
    glBindBuffer(GL_ARRAY_BUFFER, buffer_uv);

    GLfloat coordinates_UV[] = {
        // First triangle.
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        // Second triangle.
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(coordinates_UV), coordinates_UV, GL_STATIC_DRAW);

    /* Set and enable correct vertex attribute for UV coordinates (1). */
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);

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
    mat4x4_perspective(m4_projection, M_PI/4, (float)WIDTH/(float)HEIGHT,
            0.1f, 100.0f);

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
        glDrawArrays(GL_TRIANGLES, 0, SIZE(vertices_rectangle)/3);

        /* Swap. */
        glfwSwapBuffers(window);

    }
    free(texture_data);
    glfwTerminate();
}
