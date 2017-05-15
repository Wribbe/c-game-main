#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "GL/gl3w.h"
#include "GLFW/glfw3.h"

#define WIDTH 800
#define HEIGHT 600

#define M_PI 3.14159265358979323846
#define TRANSPOSE GL_TRUE


const char * source_frag_simple =\
    "#version 330 core\n"
    "\n"
    "out vec4 color;\n"
    "\n"
    "void main() {\n"
    "   color = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
    "}\n";

const char * source_vert_simple =\
    "#version 330 core\n"
    "\n"
    "layout (location = 0) in vec3 position;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "\n"
    "void main() {\n"
    "   gl_Position = projection * view * model * vec4(position, 1.0f);\n"
    "}\n";

GLfloat vertices[] = {
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.0f,  0.5f, -0.5f
};

struct v3 {
    GLfloat x;
    GLfloat y;
    GLfloat z;
};

struct v3 v3_sub(struct v3 v1, struct v3 v2)
{
    return (struct v3){v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
}

double v3_magnitude(struct v3 v)
{
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

struct v3 v3_normalize(struct v3 v)
{
    double magnitude = v3_magnitude(v);
    return (struct v3){v.x/magnitude, v.y/magnitude, v.z/magnitude};
}

struct v3 v3_cross(struct v3 v1, struct v3 v2)
{
    return (struct v3){
        v1.y*v2.z - v1.z*v2.y,
        v1.z*v2.x - v1.x*v2.z,
        v1.x*v2.y - v1.y*v2.x
    };
}

struct m4 {
    GLfloat data[4][4];
};

struct m4 m4_mul(struct m4 * A, struct m4 * B)
{
    struct m4 result = {0};
    for (size_t i=0; i<4; i++) {
        for (size_t j=0; j<4; j++) {
            for (size_t k=0; k<4; k++) {
                result.data[i][j] += A->data[i][k] * B->data[k][j];
            }
        }
    }
    return result;
}

struct m4 look_at(struct v3 * right,
           struct v3 * up,
           struct v3 * direction,
           struct v3 * position)
{
    struct m4 A = {{
        {right->x, right->y, right->z, 0.0f},
        {up->x, up->y, up->z, 0.0f},
        {direction->x, direction->y, direction->z, 0.0f},
        {0.0f, 0.0f, 0.0f, 0.0f}}
    };
    struct m4 B = {{
        {1.0f, 0.0f, 0.0f, -position->x},
        {0.0f, 1.0f, 0.0f, -position->y},
        {0.0f, 0.0f, 1.0f, -position->z},
        {0.0f, 0.0f, 0.0f, 1.0f}
    }};
    return m4_mul(&A,&B);
}

struct m4 m4_eye(void)
{
    return (struct m4) {{
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    }};
}

struct m4 m4_perspective(GLfloat near, GLfloat far, GLfloat rad_fov, GLfloat aspect_ratio)
{
    float tan_fov_half = tanf(rad_fov/2.0f);
    struct m4 perspective = m4_eye();

    perspective.data[0][0] = tan_fov_half / aspect_ratio;
    perspective.data[1][1] = tan_fov_half;
    perspective.data[2][2] = (far+near)/(near-far);
    perspective.data[2][3] = (2*far*near)/(near-far);
    perspective.data[3][2] = -1;
    perspective.data[3][3] = 0;

    return perspective;
}

void m4_translate(struct m4 * m, struct v3 v_translate)
{
    m->data[0][3] = v_translate.x;
    m->data[1][3] = v_translate.y;
    m->data[2][3] = v_translate.z;
}

const GLfloat * m4_data(struct m4 * matrix)
{
    return &(matrix->data[0][0]);
}


int main(void)
{
    GLFWwindow * window;

    if (!glfwInit()) {
        fprintf(stderr, "could not initiate glfw..\n");
        return EXIT_FAILURE;
    }

    // Set context hints.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "HELLO WORLD", NULL, NULL);
    if (!window) {
        glfwTerminate();
        fprintf(stderr, "could not initiate window..\n");
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    if (gl3wInit()) {
        fprintf(stderr, "Failed to intialize OpenGL\n");
        return EXIT_FAILURE;
    }

    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Set up vertex attribute pointers.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // Create shader variables.
    GLuint shader_vert_simple = glCreateShader(GL_VERTEX_SHADER);
    GLuint shader_frag_simple = glCreateShader(GL_FRAGMENT_SHADER);

    // Bind sources.
    glShaderSource(shader_vert_simple, 1, &source_vert_simple, NULL);
    glShaderSource(shader_frag_simple, 1, &source_frag_simple, NULL);

    // Setup error handling for compiling.
    GLint success = 0;
    size_t size_info_log = 512;
    GLchar info_log[size_info_log];

    //Compile and check shaders.
    glCompileShader(shader_vert_simple);
    glGetShaderiv(shader_vert_simple, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(shader_vert_simple, size_info_log, NULL, info_log);
        fprintf(stderr, "Could not compile vert shader: %s\n", info_log);
    }
    glCompileShader(shader_frag_simple);
    glGetShaderiv(shader_frag_simple, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(shader_frag_simple, size_info_log, NULL, info_log);
        fprintf(stderr, "Could not compile frag shader: %s\n", info_log);
    }

    // Set up shader program.
    GLuint program_shader_simple = glCreateProgram();
    glAttachShader(program_shader_simple, shader_vert_simple);
    glAttachShader(program_shader_simple, shader_frag_simple);

    // Link program.
    glLinkProgram(program_shader_simple);

    // Check linkage.
    glGetProgramiv(program_shader_simple, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(program_shader_simple, size_info_log, NULL, info_log);
        fprintf(stderr, "Could not link program: %s\n", info_log);
    }

    // Use linked program.
    glUseProgram(program_shader_simple);

    // Delete shaders.
    glDeleteShader(shader_vert_simple);
    glDeleteShader(shader_frag_simple);

    // Set up camera directions.
    struct v3 camera_position = {0.0f, 0.0f, 3.0f};
    struct v3 camera_target = {0.0f, 0.0f, 0.0f};
    struct v3 camera_direction = v3_normalize(v3_sub(camera_position, camera_target));

    struct v3 camera_up = {0.0f, 1.0f, 0.0f};
    struct v3 camera_right = v3_normalize(v3_cross(camera_up, camera_direction));

    glEnable(GL_DEPTH_TEST);

    while(!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        GLint uniform_model = glGetUniformLocation(program_shader_simple, "model");
        GLint uniform_view = glGetUniformLocation(program_shader_simple, "view");
        GLint uniform_projection = glGetUniformLocation(program_shader_simple, "projection");

        struct m4 mat_projection = m4_perspective(0.1f, 100.0f, M_PI*0.5f, (double)WIDTH/(double)HEIGHT);

        struct m4 mat_view = m4_eye();
        m4_translate(&mat_view, (struct v3){0.0f, 0.0f, -3.0f});

        struct m4 mat_model = m4_eye();

        glUniformMatrix4fv(uniform_model, 1, TRANSPOSE, m4_data(&mat_model));
        glUniformMatrix4fv(uniform_view, 1, TRANSPOSE, m4_data(&mat_view));
        glUniformMatrix4fv(uniform_projection, 1, TRANSPOSE, m4_data(&mat_projection));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
