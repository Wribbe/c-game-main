#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gl3w.h"
#include <GLFW/glfw3.h>

#include "linmath.h"

#define M_PI 3.14159265358979323846
#define UNUSED(x) (void)x
#define SIZE(x) sizeof(x)/sizeof(x[0])

#define WINDOW_WIDTH 1440
#define WINDOW_HEIGHT 900

double time_delta = 0.0f;
GLuint program_default;
GLuint indices_bound[] = {
    // Front face.
    0, 1, // Front top line.
    1, 3, // Front right line.
    3, 2, // Front bottom line.
    2, 1, // Front left line.
    // Back face.
    3, 4, // Back top line.
    4, 7, // Back right line.
    7, 6, // Back bottom line.
    6, 5, // Back left line.
};
GLuint vao_bound = 0;

GLFWwindow *
setup_glfw(void)
{
    GLFWwindow * window;

    if (!glfwInit()) {
        fprintf(stderr, "Could not initialize glfw, aborting.\n");
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "HELLO WORLD", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Could not create window, aborting.\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    /* Make window context current one. */
    glfwMakeContextCurrent(window);

    if (gl3wInit()) {
        fprintf(stderr, "Could not initialize gl3w, aborting.\n");
        exit(EXIT_FAILURE);
    }

    if (!gl3wIsSupported(3, 3)) {
        fprintf(stderr, "Profile 3.3 not supported, aborting.\n");
        exit(EXIT_FAILURE);
    }

    printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION),
            glGetString(GL_SHADING_LANGUAGE_VERSION));

    return window;
}

const GLchar * source_shader_vertex =
"#version 330 core\n"
"\n"
"uniform mat4 m4_mvp;\n"
"\n"
"layout (location=0) in vec3 vertex_data;\n"
"\n"
"void main() {\n"
"  gl_Position = m4_mvp * vec4(vertex_data, 1.0f);\n"
"}\n";

const GLchar * source_shader_fragment =
"#version 330 core\n"
"\n"
"void main() {\n"
"  gl_FragColor = vec4(1.0f);\n"
"}\n";

const GLchar * source_shader_fragment_red =
"#version 330 core\n"
"\n"
"void main() {\n"
"  gl_FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);\n"
"}\n";

const GLchar * source_shader_fragment_green =
"#version 330 core\n"
"\n"
"void main() {\n"
"  gl_FragColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);\n"
"}\n";

const GLchar * source_shader_fragment_blue =
"#version 330 core\n"
"\n"
"void main() {\n"
"  gl_FragColor = vec4(0.0f, 0.0f, 1.0f, 1.0f);\n"
"}\n";

const GLchar * source_shader_fragment_pink =
"#version 330 core\n"
"\n"
"void main() {\n"
"  gl_FragColor = vec4(1.0f, 0.8f, 0.9f, 1.0f);\n"
"}\n";

GLuint
get_program(const GLchar * source_shader_vertex,
        const GLchar * source_shader_fragment)
{
    /* Create vertex and fragment shaders. */
    GLuint shader_vertex = glCreateShader(GL_VERTEX_SHADER);
    GLuint shader_fragment = glCreateShader(GL_FRAGMENT_SHADER);

    /* Load correct source into each shader. */
    glShaderSource(shader_vertex, 1, &source_shader_vertex, NULL);
    glShaderSource(shader_fragment, 1, &source_shader_fragment, NULL);

    /* Compile shaders. */
    glCompileShader(shader_vertex);
    glCompileShader(shader_fragment);

    /* Prepare error data structures. */
    GLint operation_successful = 0;
    size_t size_error_buffer = 1024;
    GLchar buffer_error_message[size_error_buffer];

    /* Check compilation status of vertex shader. */
    glGetShaderiv(shader_vertex, GL_COMPILE_STATUS, &operation_successful);
    if (!operation_successful) {
        glGetShaderInfoLog(shader_vertex, size_error_buffer, NULL,
                buffer_error_message);
        fprintf(stderr, "Error on compiling vertex shader: %s\n\n%s\n",
                buffer_error_message, source_shader_vertex);
        return 0;
    }

    /* Check compilation status of fragment shader. */
    glGetShaderiv(shader_fragment, GL_COMPILE_STATUS, &operation_successful);
    if (!operation_successful) {
        glGetShaderInfoLog(shader_fragment, size_error_buffer, NULL,
                buffer_error_message);
        fprintf(stderr, "Error on compiling fragment shader: %s\n\n%s\n",
                buffer_error_message, source_shader_fragment);
        return 0;
    }

    /* Create shader program. */
    GLuint id_program = glCreateProgram();

    /* Attach compiled shaders. */
    glAttachShader(id_program, shader_vertex);
    glAttachShader(id_program, shader_fragment);

    /* Link the shader program. */
    glLinkProgram(id_program);

    /* Check link status. */
    glGetProgramiv(id_program, GL_LINK_STATUS, &operation_successful);
    if (!operation_successful) {
        glGetProgramInfoLog(id_program, size_error_buffer, NULL,
                buffer_error_message);
        fprintf(stderr, "Error on linking the shader program: %s\n",
                buffer_error_message);
        return 0;
    }

    /* Delete compiled shaders. */
    glDeleteShader(shader_vertex);
    glDeleteShader(shader_fragment);

    return id_program;
}

GLuint
setup_default_shader()
{
    return get_program(source_shader_vertex, source_shader_fragment);
}

GLfloat vertices[] = {
    -0.5f,  0.5f, 0.0f, // Top left.
     0.5f,  0.5f, 0.0f, // Top right.
    -0.5f, -0.5f, 0.0f, // Bottom left.
     0.5f, -0.5f, 0.0f, // Bottom right.
};

GLubyte indices[] = {
    // First triangle.
    0, // Top left.
    2, // Bottom left.
    1, // Top right.
    // Second triangle.
    1, // Top right.
    2, // Bottom left.
    3, // Bottom right.
};

GLchar *
read_file(const GLchar * filename)
{
    FILE * handle_file = fopen(filename, "r");

    if (handle_file == NULL) {
        return NULL;
    }

    /* Figure out data size. */
    fseek(handle_file, 0, SEEK_END);
    size_t size_data = ftell(handle_file);
    rewind(handle_file);

    /* Allocate enough memory to store data. */
    GLchar * buffer = malloc(size_data + 1);

    /* Read the data from file. */
    size_t size_read_data = fread(buffer, 1, size_data, handle_file);

    /* Null-terminate the data. */
    buffer[size_data] = '\0';

    fclose(handle_file);
    return buffer;
}

GLint
obj_parse_data(const GLchar * data, size_t * num_vertices, GLfloat ** ret_vertices,
        size_t * num_indices, GLuint ** ret_indices)
{
    const GLchar * current = data;
    const GLchar * newline = data;

    size_t parsed_vertices = 0;
    size_t parsed_indices = 0;

    size_t increment_vertices = 300;
    size_t increment_indices = 300;

    size_t max_vertices = 600;
    size_t max_indices = 600;

    /* Allocate memory for vertices and uv coordinates. */
    GLfloat * vertices = malloc(sizeof(GLfloat)*max_vertices);
    if (vertices == NULL) {
        fprintf(stderr, "Could not allocate memory for vertices!\n");
        return -1;
    }
    GLuint * indices = malloc(sizeof(GLuint)*max_indices);
    if (indices == NULL) {
        fprintf(stderr, "Could not allocate memory for indices!\n");
        return -1;
    }

    size_t s_tag = 3;
    GLchar tag[s_tag];

    #define S_FORMAT 256
    GLchar format_vn[S_FORMAT] = {0};
    GLchar format_v[S_FORMAT]= {0};

    const GLchar * TAG_FACE = "f";
    const GLchar * TAG_NORMAL = "vn";
    const GLchar * TAG_VERTICE = "v";

    GLfloat f1  = 0;
    GLfloat f2  = 0;
    GLfloat f3  = 0;

    GLuint f_values[3] = {0};

    snprintf(format_v, S_FORMAT, "%%*%zus %%f %%f %%f", s_tag);

    while(*current != '\0') {
        while(*newline != '\n') {
            newline++;
        }

        /* Grab the tag. */
        sscanf(current, "%3s", tag);

        if (strcmp(tag, TAG_FACE) == 0) {
            const GLchar * start = current+2; // Skip the 't ' tag.
            GLuint * current_f_value = f_values;
            for(;;) {

                /* Going to assume that there is always a vertice index. */
                int scanned = sscanf(start, "%u", current_f_value++);
                if (scanned == 0) {
                    *(current_f_value-1) = 0; // No value was parsed.
                }

                /* Loop and parse until either a / is found triplet is ended
                 * with a space. */
                int spin = 1;
                char c;
                while(spin) {
                    c = *start;
                    switch (c) {
                        case '\n':
                        case ' ':
                        case '/':
                            spin = 0;
                        break;
                        default:
                            start++;
                        break;
                    }
                }

                /* Check for tripled and end-of-line delimiters.*/
                if (c == ' ' || c == '\n') {
                    /* Handle the parsed values and reset the current_f_value
                     * pointer. */
                    if (f_values[0] != 0) { // Vertice index was parsed.
                        indices[parsed_indices++] = f_values[0]-1;
                        /* Check if there is more room, otherwise expand. */
                        if (parsed_indices >= max_indices) {
                            max_indices += increment_indices;
                            indices = realloc(indices,
                                    sizeof(GLuint)*max_indices);
                            if (indices == NULL) {
                                fprintf(stderr, "Error while expanding memory"
                                                " for the indices.\n");
                                return -1;
                            }
                        }
                    }
                    if (f_values[1] != 0) { // Texture index was parsed.
//                        printf("Texture index: %u\n",  f_values[1]);
                    }
                    if (f_values[2] != 0) { // Face normal index was parsed.
//                        printf("Normal index: %u\n",  f_values[2]);
                    }
                    current_f_value = f_values;
                }

                if (c == '\n') {
                    break; // End the outer for-loop.
                }

                /* Skip any 'delimiter' that was not a newline. */
                start++;
            }
        } else if (strcmp(tag, TAG_NORMAL) == 0) {
        } else if (strcmp(tag, TAG_VERTICE) == 0) {
            /* Asterisk in format string ignores assignment. %*3s will ignore
             * strings of length up to and including 3. */
            sscanf(current, format_v, &f1, &f2, &f3);
            /* Check if there is enough room. */
            if (parsed_vertices+3 >= max_vertices) {
                max_vertices += increment_vertices;
                vertices = realloc(vertices,
                        sizeof(GLfloat)*max_vertices);
                if (vertices == NULL) {
                    fprintf(stderr, "Error while expanding memory"
                                    " for the vertices.\n");
                    return -1;
                }
            }
            /* Put vertices in array. */
            vertices[parsed_vertices++] = f1;
            vertices[parsed_vertices++] = f2;
            vertices[parsed_vertices++] = f3;
        }


        // Increment newline and assign current to new start.
        newline++;
        current = newline;
    }

    /* Fit the vertice data. */
    if (parsed_vertices != max_vertices) {
        vertices = realloc(vertices, sizeof(GLfloat)*parsed_vertices);
        if (vertices == NULL) {
            fprintf(stderr, "Error while re-fitting memory for the vertices.\n");
            return -1;
        }
    }
    /* Set the number of parsed vertices. */
    *num_vertices = parsed_vertices;

    /* Bind the vertices back to the supplied pointer. */
    *ret_vertices = vertices;

    /* Set the number of parsed indices. */
    *num_indices = parsed_indices;

    /* Bind the indices back to the supplied pointer. */
    *ret_indices= indices;

    return 1;
}

mat4x4 m4_projection = {
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

mat4x4 m4_model = {
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


vec3 vec3_camera_up = {0.0f, 1.0f, 0.0f};
vec3 vec3_camera_direction  = {0.0f, 0.0f, -1.0f};
vec3 vec3_camera_position = {0.0f, 0.0f, 13.0f};

GLfloat speed_camera = 10.0f;

double mouse_x_prev = (float)WINDOW_WIDTH/2.0f;
double mouse_y_prev = (float)WINDOW_HEIGHT/2.0f;
GLboolean input_mouse_first = GL_TRUE;

double mouse_pitch = 0;
double mouse_yaw = 0;

double two_pi = 2*M_PI;


GLboolean key_down[512] = {0};

static void
callback_mouse_position_method(GLFWwindow * w, double x, double y)
{
    if (input_mouse_first) {
        glfwSetCursorPos(w, mouse_x_prev, mouse_y_prev);
        input_mouse_first = GL_FALSE;
        return;
    }

    double diff_x = x - mouse_x_prev;
    double diff_y = mouse_y_prev - y;

    double mouse_sensitivity = 0.0005f;

    diff_x *= mouse_sensitivity;
    diff_y *= mouse_sensitivity;

    mouse_pitch += diff_y;
    mouse_yaw += diff_x;

    mouse_x_prev = x;
    mouse_y_prev = y;

    /* diff_x = 0, and diff_y = 0 should result in {0,0,-1} direction. */
    vec3_camera_direction[0] = cos(mouse_pitch) * sin(mouse_yaw);
    vec3_camera_direction[1] = sin(mouse_pitch);
    vec3_camera_direction[2] = -cos(mouse_pitch) * cos(mouse_yaw);
}

static void
callback_key_method(GLFWwindow * window, int key, int scancode, int action,
        int mods)
{

    if (action == GLFW_REPEAT) {
        return;
    }

    UNUSED(mods);
    UNUSED(scancode);

    if (key_down[GLFW_KEY_ESCAPE]) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    } else {
        key_down[key] = action == GLFW_PRESS ? GL_TRUE : GL_FALSE;
    }
}

void
process_frame_events(void)
{
    GLfloat speed_camera_delta = speed_camera * time_delta;

    vec3 vec3_forward = {0};
    vec3_scale(vec3_forward, vec3_camera_direction, speed_camera_delta);

    vec3 vec3_right = {0};
    vec3_mul_cross(vec3_right, vec3_camera_direction, vec3_camera_up);
    vec3_norm(vec3_right, vec3_right);
    vec3_scale(vec3_right, vec3_right, speed_camera_delta);
    if (key_down[GLFW_KEY_W]) { // Go forward in camera direction.
        vec3_add(vec3_camera_position, vec3_camera_position,
                vec3_forward);
    }
    if (key_down[GLFW_KEY_S]) { // Go backward in camera direction.
        vec3_sub(vec3_camera_position, vec3_camera_position,
                vec3_forward);
    }
    if (key_down[GLFW_KEY_D]) { // Go along right direction.
        vec3_add(vec3_camera_position, vec3_camera_position,
                vec3_right);
    }
    if (key_down[GLFW_KEY_A]) { // Go along opposite to right direction.
        vec3_sub(vec3_camera_position, vec3_camera_position,
                vec3_right);
    }
}

struct object_drawable {
    GLuint vao;
    size_t num_vertices;
    size_t num_indices;
    size_t num_normals;
    GLfloat * vertices;
    GLfloat * normals;
    GLuint * indices;
};

#define MAX_DRAWABLE_OBJECTS 300
struct object_drawable r_objects_drawable[MAX_DRAWABLE_OBJECTS] = {0};
size_t object_drawable_first_empty = 1; // 0 used for errors.

struct object {
    struct object_drawable * drawable;
    mat4x4 model;
    vec3 velocity;
    GLuint program;
    vec3 bounds[8];
};

#define MAX_OBJECTS 900
struct object r_objects[MAX_OBJECTS] = {0};
size_t object_first_empty = 1; // 0 used for errors.

GLuint
get_id_drawable_object(void)
{
    if (object_drawable_first_empty >= MAX_DRAWABLE_OBJECTS) {
        return 0;
    }
    GLuint id = object_drawable_first_empty++;
    /* Set default program. */
    r_objects[id].program = program_default;
    /* Set model to unity matrix. */
    for (size_t i=0; i<4; i++) {
        r_objects[id].model[i][i] = 1.0f;
    }
    return id;
}

GLuint
get_id_object(void)
{
    if (object_first_empty >= MAX_OBJECTS) {
        return 0;
    }
    return object_first_empty++;
}

void
object_drawable_setup_values(struct object_drawable * drawable,
        size_t num_vertices,
        size_t num_indices,
        size_t num_normals,
        GLfloat * vertices,
        GLuint * indices,
        GLfloat * normals)
{
    /* Set drawable parameters and bind pointers. */
    drawable->num_vertices = num_vertices;
    drawable->num_indices = num_indices;
    drawable->num_normals = num_normals;

    drawable->vertices = vertices;
    drawable->indices = indices;
    drawable->normals = NULL;
}

void
object_drawable_setup_buffers(struct object_drawable * drawable)
{
    GLuint vao = 0;
    GLuint vbo = 0;

    /* Unbind any currently bound vertex-array and vertex-buffer object. */
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    /* Setup vertex array object. */
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    drawable->vao = vao;

    /* Setup vertex buffer object. */
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    /* Populate bound array_buffer with vertice data. */
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*drawable->num_vertices,
            drawable->vertices, GL_STATIC_DRAW);

    /* Setup vertex attribute pointer. */
    GLuint attribute_vertex_data = 0;
    glVertexAttribPointer(
            attribute_vertex_data, // Attribute index.
            3,          // Number of elements per vertex.
            GL_FLOAT,   // Data type of element.
            GL_FALSE,   // Specify if data is normalized.
            0,          // Stride = 0 -> elements tightly packed.
            0           // Offset-pointer to first element.
    );
    /* Enable vertex attribute pointer. */
    glEnableVertexAttribArray(attribute_vertex_data);

    /* Create buffer for indices. */
    GLuint vbo_obj_indices = 0;
    glGenBuffers(1, &vbo_obj_indices);

    /* Populate indices buffer. */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_obj_indices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*drawable->num_indices,
            drawable->indices, GL_STATIC_DRAW);

    /* Unbind all buffers. */
    glBindVertexArray(0); // Important that this is done first!
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


void
object_create_bounds(struct object * object)
{
    GLfloat * vertices = object->drawable->vertices;
    GLfloat * stop = vertices+object->drawable->num_vertices;

    GLfloat max_x = *vertices++;
    GLfloat min_x = max_x;

    GLfloat max_y = *vertices++;
    GLfloat min_y = max_y;

    GLfloat max_z = *vertices++;
    GLfloat min_z = max_z;

    for(; vertices < stop; vertices += 3) {
        GLfloat x = *vertices;
        GLfloat y = *vertices+1;
        GLfloat z = *vertices+2;

        if (x > max_x) {
            max_x = x;
        }
        if (x < min_x) {
            min_x = x;
        }

        if (y > max_y) {
            max_y = y;
        }
        if (y < min_y) {
            min_y = y;
        }

        if (z > max_z) {
            max_z = z;
        }
        if (z < min_z) {
            min_z = z;
        }

    }
    vec3 bounds[] = {
        {min_x, max_y, min_z}, // #0 Front Top Left.
        {max_x, max_y, min_z}, // #1 Front Top Right.
        {min_x, min_y, min_z}, // #2 Front Bottom Left.
        {max_x, min_y, min_z}, // #3 Front Bottom Right.
        {min_x, max_y, max_z}, // #4 Back Top Left.
        {max_x, max_y, max_z}, // #5 Back Top Right.
        {min_x, min_y, max_z}, // #6 Back Bottom Left.
        {max_x, min_y, max_z}, // #7 Back Bottom Right.
    };
    memcpy(object->bounds, bounds, 8*sizeof(vec3));
}

GLuint
object_from_obj(const GLchar * filename)
{
    //const GLchar * filename = "cube.obj";
    GLchar * str_obj = read_file(filename);
    if (str_obj == NULL) {
        return 0;
    }

    size_t id_object = get_id_object();
    if (id_object < 1) {
        return id_object;
    }

    struct object * object = &r_objects[id_object];

    size_t id_drawable = get_id_drawable_object();
    if (id_drawable < 1) { // No objects left.
        object->drawable = NULL;
        return id_object;
    }

    struct object_drawable * drawable = &r_objects_drawable[id_drawable];
    object->drawable = drawable;

    GLfloat * obj_vertices = NULL;
    size_t num_obj_vertices = 0;
    GLuint * obj_indices = NULL;
    size_t num_obj_indices = 0;

    GLint status = obj_parse_data(str_obj, &num_obj_vertices, &obj_vertices,
            &num_obj_indices, &obj_indices);
    if (status < 0) {
        fprintf(stderr,
                "Something went wrong with the obj-parsing, aborting.\n");
        return EXIT_FAILURE;
    }
    free(str_obj);

    /* Set variables and bind data pointers for drawable. */
    object_drawable_setup_values(drawable,
            num_obj_vertices,
            num_obj_indices,
            0,
            obj_vertices,
            obj_indices,
            NULL);

    /* Set up correct buffer structures and vao for binding. */
    object_drawable_setup_buffers(drawable);

    /* Set up bounds. */
    object_create_bounds(object);

    /* Return id. */
    return id_object;
}

GLuint vbo_bound_vertices = 0;

void
draw_object(GLuint id_object)
{
    struct object * object = &r_objects[id_object];
    glBindVertexArray(object->drawable->vao);
    glUseProgram(object->program);
    glDrawElements(GL_TRIANGLES, object->drawable->num_indices, GL_UNSIGNED_INT, NULL);
    glBindVertexArray(vao_bound);
    glDrawElements(GL_LINES, SIZE(indices_bound), GL_UNSIGNED_INT, NULL);
    glBindVertexArray(0);
}

GLuint
object_create_plane(GLfloat width, GLfloat height)
{
    GLuint id_object = get_id_object();
    if (id_object < 1) {
        return id_object;
    }

    struct object * object = &r_objects[id_object];

    GLuint id_drawable = get_id_drawable_object();
    if (id_drawable < 1) { // No objects left.
        object->drawable = NULL;
        return id_object;
    }

    struct object_drawable * drawable = &r_objects_drawable[id_drawable];
    object->drawable = drawable;

    GLfloat hw = width/2.0f;
    GLfloat hh = height/2.0f;

    GLfloat vertices[] = {
        -hw, 0.0f, -hh, // Top left.
         hw, 0.0f, -hh, // Top right.
        -hw, 0.0f,  hh, // Bottom left.
         hw, 0.0f,  hh, // Bottom right.
    };

    GLuint indices[] = {
        0, 2, 1, // First CCW triangle.
        1, 2, 3, // Second CCW triangle.
    };

    /* Allocate memory for vertices and indices. */
    GLfloat * data_vertices = malloc(sizeof(vertices));
    if (data_vertices == NULL) {
        fprintf(stderr,
                "Could not allocate data for vertices when creating plane.\n");
        return 0;
    }
    memcpy(data_vertices, vertices, sizeof(vertices));

    GLuint * data_indices = malloc(sizeof(indices));
    if (data_indices == NULL) {
        fprintf(stderr,
                "Could not allocate data for indices when creating plane.\n");
        return 0;
    }
    memcpy(data_indices, indices, sizeof(indices));

    object_drawable_setup_values(drawable,
            SIZE(vertices),
            SIZE(indices),
            0,
            data_vertices,
            data_indices,
            NULL);
    object_drawable_setup_buffers(drawable);

    /* Set up bounds. */
    object_create_bounds(object);

    return id_object;
}

void
object_set_program(GLuint id_object, GLuint id_program)
{
    r_objects[id_object].program = id_program;
}

void
object_translate_to(mat4x4 m, GLuint id_object, vec3 direction)
{
    GLfloat * model = r_objects[id_object].model[0];
    for (size_t i=0; i<3; i++) {
        m[3][i] = model[12+i] + direction[i];
    }
}

void
object_translate(GLuint id_object, vec3 direction)
{
    mat4x4 * m = &r_objects[id_object].model;
    object_translate_to(*m, id_object, direction);
}

void
object_rotate_x(GLuint id_object, GLfloat angle)
{
    mat4x4 * m = &r_objects[id_object].model;
    mat4x4_rotate_X(*m, *m, angle);
}

void
object_rotate_y(GLuint id_object, GLfloat angle)
{
    mat4x4 * m = &r_objects[id_object].model;
    mat4x4_rotate_Y(*m, *m, angle);
}

void
object_rotate_z(GLuint id_object, GLfloat angle)
{
    mat4x4 * m = &r_objects[id_object].model;
    mat4x4_rotate_Z(*m, *m, angle);
}

void
object_velocity_set(GLuint id_object, vec3 velocity)
{
    for (size_t i=0; i<3; i++) {
        r_objects[id_object].velocity[i] = velocity[i];
    }
}

void
object_velocity_add(GLuint id_object, vec3 velocity)
{
    for (size_t i=0; i<3; i++) {
        r_objects[id_object].velocity[i] += velocity[i];
    }
}

void
object_velocity_tick(GLuint id_object)
{
    vec3 scaled_velocity = {0};
    vec3_scale(scaled_velocity, r_objects[id_object].velocity, time_delta);
    object_translate(id_object, scaled_velocity);
}

int
main(void)
{
    GLFWwindow * window = setup_glfw();
    program_default = setup_default_shader();

    GLuint program_red = get_program(source_shader_vertex,
            source_shader_fragment_red);
    GLuint program_green = get_program(source_shader_vertex,
            source_shader_fragment_green);
    GLuint program_blue = get_program(source_shader_vertex,
            source_shader_fragment_blue);
    GLuint program_pink = get_program(source_shader_vertex,
            source_shader_fragment_pink);

    const GLchar * filename_obj = "cube.obj";
    GLuint id_cube = object_from_obj(filename_obj);
    if (id_cube < 1) {
        fprintf(stderr, "Error on loading %s from disk, aborting.\n",
                filename_obj);
        return EXIT_FAILURE;
    }
    object_set_program(id_cube, program_pink);

    filename_obj = "suzanne.obj";
    GLuint id_suzanne = object_from_obj(filename_obj);
    if (id_suzanne < 1) {
        fprintf(stderr, "Error on loading %s from disk, aborting.\n",
                filename_obj);
        return EXIT_FAILURE;
    }
    object_set_program(id_suzanne, program_green);

    filename_obj = "sphere.obj";
    GLuint id_sphere = object_from_obj(filename_obj);
    if (id_sphere < 1) {
        fprintf(stderr, "Error on loading %s from disk, aborting.\n",
                filename_obj);
        return EXIT_FAILURE;
    }
    object_set_program(id_sphere, program_blue);

    GLuint id_plane = object_create_plane(4.0f, 4.0f);
    if (id_plane < 1) {
        fprintf(stderr, "Error creating plane, aborting.\n");
        return EXIT_FAILURE;
    }
    object_set_program(id_plane, program_red);

    GLuint id_floor = object_create_plane(10.0f, 10.0f);
    if (id_floor < 1) {
        fprintf(stderr, "Error creating floor, aborting.\n");
        return EXIT_FAILURE;
    }
    object_set_program(id_floor, program_blue);
    object_translate(id_floor, (vec3){0.0f, -4.0f, 0.0f});

    object_translate(id_plane, (vec3){1.0f, 1.0f, 1.0f});
    object_translate(id_cube, (vec3){2.4f, -1.3f, 0.0f});
    object_translate(id_suzanne, (vec3){0.0f, 0.0f, 3.0f});
    object_translate(id_sphere, (vec3){-2.0f, -1.0f, 0.0f});

    /* Set object velocity. */
    GLfloat force_gravity = 0.5f;
    object_velocity_set(id_plane, (vec3){0.0f, -force_gravity, 0.0f});
    object_velocity_set(id_cube, (vec3){0.0f, -force_gravity, 0.0f});
    object_velocity_set(id_suzanne, (vec3){0.0f, -force_gravity, 0.0f});
    object_velocity_set(id_sphere, (vec3){0.0f, -force_gravity, 0.0f});

    glfwSetKeyCallback(window, callback_key_method);
    glfwSetCursorPosCallback(window, callback_mouse_position_method);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    /* Set projection matrix as a perspective matrix. */
    mat4x4_perspective(m4_projection, M_PI/4,
            (float)WINDOW_WIDTH/(float)WINDOW_HEIGHT, 0.1f, 100.0f);

    double time_prev = glfwGetTime();
    double time_now = 0;

    /* Setup bounding box display vao. */
    glGenVertexArrays(1, &vao_bound);
    glBindVertexArray(vao_bound);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    /* Bind and fill GL_ELEMENT_ARRAY_BUFFER. */
    GLuint vbo_indice_bound = 0;
    glGenBuffers(1, &vbo_indice_bound);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_indice_bound);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_bound), indices_bound,
            GL_STATIC_DRAW);

    /* Create buffer handle for dumping individual bound box data. */
    glGenBuffers(1, &vbo_bound_vertices);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_bound_vertices);


    /* Unbind vertex array. */
    glBindVertexArray(0);

    /* Unbind buffers. */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /* Enable depth testing. */
    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window)) {

        time_now = glfwGetTime();
        time_delta = time_now - time_prev;
        time_prev = time_now;

        /* Clear color and depth buffers. */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* Poll events. */
        glfwPollEvents();
        process_frame_events();

        /* Rotate objects. */

//        object_rotate_x(id_plane, M_PI/2*time_delta);
//        object_rotate_z(id_plane, M_PI/2*time_delta);
//
//        object_rotate_y(id_suzanne, M_PI/8*time_delta);
//
//        object_rotate_x(id_cube, M_PI/3*time_delta);
//        object_rotate_z(id_cube, M_PI/2*time_delta);

        /* Re-calculate view matrix. */
        vec3 vec3_camera_center = {0};
        vec3_add(vec3_camera_center, vec3_camera_position, vec3_camera_direction);
        mat4x4_look_at(m4_view, vec3_camera_position, vec3_camera_center,
                vec3_camera_up);

        /* Re-calculate mvp matrix for all objects. */
        for (size_t i=1; i<object_first_empty; i++) {

            struct object * object = &r_objects[i];

            /* Tick velocity. */
            object_velocity_tick(i);

            mat4x4_mul(m4_mvp, m4_view, object->model);
            mat4x4_mul(m4_mvp, m4_projection, m4_mvp);

            glUseProgram(object->program);
            GLuint ulocation_m4_mvp = glGetUniformLocation(object->program,
                    "m4_mvp");

            /* Supply mvp matrix to vertex shader. */
            glUniformMatrix4fv(
                    ulocation_m4_mvp,
                    1,
                    GL_FALSE,
                    m4_mvp[0]);

            /* Draw. */
            draw_object(i);
        }


        /* Swap. */
        glfwSwapBuffers(window);

        GLenum err;
        while((err = glGetError()) != GL_NO_ERROR) {
            printf("GLerror: 0x%x.\n", err);
        }
    }
    glfwTerminate();
}
