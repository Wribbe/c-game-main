#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "gl3w.h"
#include "linmath.h"
#include <GLFW/glfw3.h>

#define M_PI 3.14159265358979323846
#define UNUSED(x) (void)x

GLchar *
read_file(const char * filepath)
{
    FILE * fpointer =  fopen(filepath, "r");
    if (fpointer == NULL) {
        fprintf(stderr, "Could not find file %s, aborting.\n", filepath);
        exit(EXIT_FAILURE);
    }

    fseek(fpointer, 0, SEEK_END);
    size_t size = ftell(fpointer);
    rewind(fpointer);

    GLchar * data = malloc(size+1);
    if (data == NULL) {
        fprintf(stderr, "Could not allocate memory for reading file,"
                " aborting.\n");
        exit(EXIT_FAILURE);
    }

    fread(data, size, 1, fpointer);
    data[size] = '\0';

    return data;
}

GLuint
create_shader_program(const char * vertex_source_path,
                      const char * fragment_source_path)
{
    GLchar * source_vertex = read_file(vertex_source_path);
    GLchar * source_fragment = read_file(fragment_source_path);

    GLuint shader_vert = glCreateShader(GL_VERTEX_SHADER);
    GLuint shader_frag = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(shader_vert, 1, (const GLchar **)&source_vertex, NULL);
    glShaderSource(shader_frag, 1, (const GLchar **)&source_fragment, NULL);

    size_t size_buffer = 1024;
    char buffer_log[size_buffer];
    GLint success = 0;

    glCompileShader(shader_vert);
    glGetShaderiv(shader_vert, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        glGetShaderInfoLog(shader_vert, size_buffer, NULL, buffer_log);
        fprintf(stderr,
                "Compilation of vertex shader failed, aborting: \n%s\n",
                buffer_log);
        fprintf(stderr, "Shader data:\n\n%s\n", source_vertex);
        exit(EXIT_FAILURE);
    }

    glCompileShader(shader_frag);
    glGetShaderiv(shader_frag, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        glGetShaderInfoLog(shader_frag, size_buffer, NULL, buffer_log);
        fprintf(stderr,
                "Compilation of fragment shader failed, aborting: \n%s\n",
                buffer_log);
        fprintf(stderr, "Shader data:\n\n%s\n", source_fragment);
        exit(EXIT_FAILURE);
    }

    GLuint program = glCreateProgram();

    glAttachShader(program, shader_vert);
    glAttachShader(program, shader_frag);

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (success != GL_TRUE) {
        glGetProgramInfoLog(program, size_buffer, NULL, buffer_log);
        fprintf(stderr,
                "Linking of program failed, aborting: \n%s\n",
                buffer_log);
        exit(EXIT_FAILURE);
    }

    glDeleteShader(shader_vert);
    glDeleteShader(shader_frag);

    free(source_vertex);
    free(source_fragment);

    return program;
}

size_t
get_data(const char * filepath, GLfloat ** float_data)
{
    char * data = read_file(filepath);
    size_t num_values = 0;
    char * pointer = data;
    for (;*pointer != '\0'; pointer++) {
        if (*pointer == ',') {
            num_values++;
        }
    }

    *float_data = malloc(num_values*sizeof(GLfloat));
    if (*float_data == NULL) {
        fprintf(stderr,
                "Could not allocate enough memory for float data, "
                "aborting.\n");
        exit(EXIT_FAILURE);
    }

    char * data_pointer = data;
    float * current_float = *float_data;

    for(;;data_pointer++) {
        char c = *data_pointer;
        if (c == '\0') {
            break;
        }
        if(c != '-' && c != '+' && !isdigit(c)) {
            continue;
        }
        *current_float++ = strtof(data_pointer, &data_pointer);
    }

    free(data);

    return num_values;
}

size_t
load_bmp(const char * filepath, unsigned char ** data, size_t * width, size_t * height)
{
    FILE * fp = fopen(filepath, "rb");
    if (fp == NULL) {
        fprintf(stderr, "No such file %s, aborting.\n", filepath);
        exit(EXIT_FAILURE);
    }

    // Read the header of the BMP file.
    size_t size_header = 54;
    unsigned char header[size_header];
    size_t bytes_read = fread(header, 1, size_header, fp);
    if (bytes_read != size_header) {
        fprintf(stderr,
                "Mismatch header size, read: %zu != header: %zu, aborting.\n",
                bytes_read, size_header);
        exit(EXIT_FAILURE);
    }

    // Check first bytes of header file.
    if (header[0] != 'B' || header[1] != 'M') {
        fprintf(stderr, "Not a correct BMP file, aborting.\n");
        exit(EXIT_FAILURE);
    }

    // Extract data from header.
    uint32_t pos_data = *(uint32_t*)&(header[0x0A]);
    uint32_t size_image = *(uint32_t*)&(header[0x22]);
    uint32_t image_width = *(uint32_t*)&(header[0x12]);
    uint32_t image_height = *(uint32_t*)&(header[0x16]);

    // Correct any missing information.
    if (size_image == 0) {
        size_image = image_width*image_height*3;
    }
    if (pos_data == 0) {
        pos_data = size_header;
    }

    // Allocate memory for image data.
    *data = malloc(size_image*sizeof(char));
    if (*data == NULL) {
        fprintf(stderr,
                "Not enough memory to allocate BMP data for image %s, "
                " aborting.\n", filepath);
        exit(EXIT_FAILURE);
    }

    // Read data to allocated space.
    fread(*data, 1, size_image, fp);
    // Close file.
    fclose(fp);

    // Write width and height if specified.
    if (width != NULL) {
        *width = image_width;
    }
    if (height != NULL) {
        *height = image_height;
    }

    return size_image;
}

void
vec3_copy(vec3 * to, vec3 * from)
{
    *to[0] = *from[0];
    *to[1] = *from[1];
    *to[2] = *from[2];
}

void
vec3_add_float(vec3 * v, float * f)
{
    *v[0] += *f;
    *v[1] += *f;
    *v[2] += *f;
}

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

    GLuint program = create_shader_program("vert_texture.glsl",
                                           "frag_texture.glsl");
    glUseProgram(program);
    GLfloat * data_vertices = NULL;
    size_t num_vertices = get_data("cube.dat", &data_vertices);

    GLuint array_buffer;
    glGenVertexArrays(1, &array_buffer);
    glBindVertexArray(array_buffer);

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, num_vertices*sizeof(GLfloat), data_vertices,
            GL_STATIC_DRAW);

    GLuint layout_vPosition = 0;
    glVertexAttribPointer(layout_vPosition, 3, GL_FLOAT, GL_FALSE, 0,
            (void*)0);
    glEnableVertexAttribArray(layout_vPosition);

    // Projection matrix; FOV: PI/4, RATIO: 4:3 , RANGE: 0.1 -> 100 units.
    mat4x4 m4_projection = {0};
    mat4x4_perspective(m4_projection, M_PI/4, (float)WIDTH/(float)HEIGHT, 0.1f,
            100.f);

    // Camera matrix.
    mat4x4 m4_view = {0};
    mat4x4_look_at(m4_view, (vec3){4,3,3}, (vec3){0,0,0}, (vec3){0,1,0});

    // Model matrix.
    mat4x4 m4_model = {0};
    mat4x4_identity(m4_model);

    // MVP matrix.
    mat4x4 m4_mvp = {0};
    mat4x4_mul(m4_mvp, m4_view, m4_model);
    mat4x4_mul(m4_mvp, m4_projection, m4_mvp);

    GLuint uniform_mvp = glGetUniformLocation(program, "mvp");
    glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, &m4_mvp[0][0]);

    // Get color data.
    GLfloat * data_color = NULL;
    size_t num_color_vertices = get_data("cube_colors.dat", &data_color);

    // Create buffer for color data.
    GLuint buffer_color;
    glGenBuffers(1, &buffer_color);
    glBindBuffer(GL_ARRAY_BUFFER, buffer_color);
    glBufferData(GL_ARRAY_BUFFER, num_color_vertices*sizeof(GLfloat),
            data_color, GL_STATIC_DRAW);

    // Setup attribute for color.
    GLuint loc_vertex_color = 1;
    glEnableVertexAttribArray(loc_vertex_color);
    glVertexAttribPointer(loc_vertex_color, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // Enable depth test.
    glEnable(GL_DEPTH_TEST);
    // Accept fragments that are closer to the camera than other ones.
    glDepthFunc(GL_LESS);

    // Read texture-data from file.
    const char * filepath_texture = "uvtemplate.bmp";
    unsigned char * data_bmp = NULL;
    size_t bmp_widht, bmp_height;
    size_t size_data_bmp = load_bmp(filepath_texture, &data_bmp, &bmp_widht,
            &bmp_height);
    printf("Got bmp data of size: %zu\n", size_data_bmp);

    // Create one OpenGL texture.
    GLuint texture_bmp;
    glGenTextures(1, &texture_bmp);

    // Bind created texture.
    glBindTexture(GL_TEXTURE_2D, texture_bmp);
    // Give the texture data to OpenGL.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bmp_widht, bmp_height, 0, GL_BGR,
            GL_UNSIGNED_BYTE, data_bmp);
    // Set up texture properties.

    // Generate mipmaps.
    glGenerateMipmap(GL_TEXTURE_2D);
    // Magnifying beyond largest available mapmap -> use linear filtering.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Minifying, blend two mipmaps linearly.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // Load uv-coordinates.
    GLfloat * data_uv_coords = NULL;
    size_t num_uv_coords = get_data("cube_UV.dat", &data_uv_coords);

    // Generate buffer for UV-coordinates.
    GLuint buffer_uv;
    glGenBuffers(1, &buffer_uv);
    // Bind buffer and load data.
    glBindBuffer(GL_ARRAY_BUFFER, buffer_uv);
    glBufferData(GL_ARRAY_BUFFER, num_uv_coords*sizeof(GLfloat),
            data_uv_coords, GL_STATIC_DRAW);
    GLuint layout_vUV = 1;
    glVertexAttribPointer(layout_vUV, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(layout_vUV);

    // Get texture sampler position.
    GLuint uniform_textureSampler = glGetUniformLocation(program,
            "textureSampler");

    // Activate texture.
    glActiveTexture(GL_TEXTURE0);
    // Bind texture.
    glBindTexture(GL_TEXTURE_2D, texture_bmp);
    // Set sampler to use texture unit 0.
    glUniform1i(uniform_textureSampler, 0);

    // Set up position, angels and FoV for view.
    vec3 position = {0.0f, 0.0f, 5.0f};
    float angle_horizontal = M_PI;
    float angle_vertical = 0.0f;
    float initial_fov = 0.5f*M_PI;

    // Set up speed.
    float speed_general = 3.0f;
    float speed_mouse = 0.005f;

    // Get mouse position.
    double xpos = 0;
    double ypos = 0;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Set mouse position.
    double half_width = (double)WIDTH/2.0f;
    double half_height = (double)HEIGHT/2.0f;

    double time_current = glfwGetTime();
    double time_previous = glfwGetTime();
    float time_delta = 0.0f;

    glfwSetCursorPos(window, half_width, half_height);

    // Compute new orientation.
    angle_horizontal += speed_mouse * time_delta * half_width;
    angle_vertical += speed_mouse * time_delta * half_height;

    vec3 direction = {
        cos(angle_vertical) * sin(angle_horizontal),
        sin(angle_vertical),
        cos(angle_vertical) * cos(angle_horizontal)
    };

    vec3 right = {
        sin(angle_horizontal - M_PI*0.5f),
        0,
        cos(angle_horizontal - M_PI*0.5f)
    };

    vec3 up = {0};
    vec3_mul_cross(up, right, direction);

    /* Set key callback function for main window. */
    glfwSetKeyCallback(window, key_callback);



    /* Loop until the user closes window. */
    while (!glfwWindowShouldClose(window)) {

        time_current = glfwGetTime();
        time_delta = time_current - time_previous;

        /* Render. */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* Draw. */
        glDrawArrays(GL_TRIANGLES, 0, num_vertices/3);

        /* Swap. */
        glfwSwapBuffers(window);

        /* Poll events. */
        glfwPollEvents();

        /* Input actions. */
        vec3 mod_direction = {0};

        time_previous = time_current;
    }

    glfwTerminate();

    free(data_vertices);
    free(data_color);
    free(data_bmp);
    free(data_uv_coords);

    return EXIT_SUCCESS;
}
