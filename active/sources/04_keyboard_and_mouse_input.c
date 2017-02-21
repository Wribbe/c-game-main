#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "glad.h"
#include <GLFW/glfw3.h>

#define UNUSED(x) (void)x;

/* Global variables. */
#define WIDTH 800
#define HEIGHT 600

/* Global storage. */
double m_xpos = 0;
double m_ypos = 0;

void prefixed_output(FILE * output,
                     const char * tag,
                     const char * message)
{
    fprintf(output, "%s %s\n", tag, message);
}

void error_and_exit(const char * message)
{
    const char * tag = "[!]";
    prefixed_output(stderr, tag, message);
    exit(EXIT_FAILURE);
}

void glfw_set_context(void)
    /* OpenGL window context hints. */
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

const GLchar * source_vert_basic = \
    "#version 330 core\n"
    "layout (location=0) in vec3 position;\n"
    "layout (location=1) in vec3 color_input;\n"
    "\n"
    "out vec4 vertex_color;\n"
    "\n"
    "void main() {\n"
    "   gl_Position = vec4(position, 1.0f);\n"
    "   vertex_color = vec4(color_input, 1.0f);\n"
    "}\n";


const GLchar * source_frag_basic = \
    "#version 330 core\n"
    "in vec4 vertex_color;\n"
    "out vec4 color;\n"
    "\n"
    "void main() {\n"
    "   color = vertex_color;\n"
    "}\n";


GLfloat vertex_data_triangle[] = \
    {
        0.0f,  1.0f,  0.0f, 1.0f, 0.0f, 0.0f,
       -1.0f, -1.0f,  0.0f, 0.0f, 1.0f, 0.0f,
        1.0f, -1.0f,  0.0f, 0.0f, 0.0f, 1.0f,
    };

void callback_simple_keyboard(GLFWwindow * window,
                              int key,
                              int scancode,
                              int action,
                              int mods)
    /* Simple callback function for keyboard input. */
{
    UNUSED(scancode);
    UNUSED(mods);
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void callback_simple_mouse(GLFWwindow * window,
                           int button,
                           int action,
                           int mods)
    /* Simple callback function for mouse input. */
{
    UNUSED(mods);
    UNUSED(window);
    const char * button_pressed = "";
    const char * button_action = "";
    switch (action) {
        case GLFW_PRESS:
            button_action = "pressed";
            break;
        case GLFW_RELEASE:
            button_action = "released";
            break;
        default:
            button_action = "UNKNOWN STATE.";
            break;
    }
    switch(button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            button_pressed = "LEFT";
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            button_pressed = "RIGHT";
            break;
        default:
            button_pressed = "UNKNOWN";
            break;
    }
    const char * fmt_msg = "Mouse button <%s> %s @ (%.2f, %.2f).\n";
    printf(fmt_msg, button_pressed, button_action, m_xpos, m_ypos);
}

void callback_simple_cursor_pos(GLFWwindow * window, double xpos, double ypos)
    /* Simple callback function for mouse position. */
{
    UNUSED(window);
    m_xpos = xpos/WIDTH;
    m_ypos = ypos/HEIGHT;
}

int main(int argc, char ** argv)
{
    if (!glfwInit()) {
        error_and_exit("Could not initialize GLFW, aborting.\n");
    }

    UNUSED(argc);

    glfw_set_context();

    /* Extract the window title from the input arguments. */
    char * filename = argv[0];
    size_t filename_length = strlen(filename);
    size_t title_max_len = filename_length + 1;
    char window_title[title_max_len];

    size_t i=0;
    char * end = filename+filename_length;
    char * start = NULL;
    for (i=filename_length; i>0; i--) {
        char current = filename[i];
        if (current == '/' && start == NULL) {
            start = filename+i+1;
        }
    }
    char * pointer = NULL;
    char * window_pointer = window_title;
    for (pointer = start; pointer != end; pointer++, window_pointer++) {
        *window_pointer = *pointer;
    }
    *window_pointer = '\0';

    GLFWwindow * window = glfwCreateWindow(WIDTH,
                                           HEIGHT,
                                           window_title,
                                           NULL,
                                           NULL);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    /* Set window callback function. */
    glfwSetKeyCallback(window, callback_simple_keyboard);
    glfwSetMouseButtonCallback(window, callback_simple_mouse);
    glfwSetCursorPosCallback(window, callback_simple_cursor_pos);

    // ========================================
    // == Buffers
    // ========================================

    /* Create buffer variables. */
    GLuint VBO = 0;
    GLuint VAO = 0;

    /* Generate empty buffer objects. */
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);

    /*Bind vertex buffer. */
    glBindVertexArray(VAO);

    /* Bind array buffer. */
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    /* Populate buffer with data. */
    size_t size_data = sizeof(vertex_data_triangle);
    GLuint draw_type = GL_STATIC_DRAW;
    glBufferData(GL_ARRAY_BUFFER,
                 size_data,
                 vertex_data_triangle,
                 draw_type);
    /* Set and enable vert attribute pointer 0 for position. */
    size_t stride_attrib = 6*sizeof(GLfloat);
    glVertexAttribPointer(0,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          stride_attrib,
                          (GLvoid*)0);
    glEnableVertexAttribArray(0);
    /* Enable input_color vertex array. */
    size_t color_offset = 3*sizeof(GLfloat);
    glVertexAttribPointer(1,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          stride_attrib,
                          (GLvoid*)color_offset);
    glEnableVertexAttribArray(1);
    /* Unbind buffer objects. */
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // ========================================
    // == Shaders
    // ========================================

    /* Create vertex and fragment shader variables. */
    GLuint sh_vert_basic = glCreateShader(GL_VERTEX_SHADER);
    GLuint sh_frag_basic = glCreateShader(GL_FRAGMENT_SHADER);
    /* Bind shader sources. */
    glShaderSource(sh_vert_basic, 1, &source_vert_basic, 0);
    glShaderSource(sh_frag_basic, 1, &source_frag_basic, 0);
    /* Set up compilation info storage. */
    GLint success = 0;
    size_t size_buffer_info = 1024;
    GLchar buffer_info[size_buffer_info];
    /* Compile shaders. */
    glCompileShader(sh_vert_basic);
    glGetShaderiv(sh_vert_basic, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(sh_vert_basic,
                           size_buffer_info,
                           NULL,
                           buffer_info);
        fprintf(stderr, "Compilation of vertex shader failed: %s\n",
                buffer_info);
    }
    glCompileShader(sh_frag_basic);
    glGetShaderiv(sh_frag_basic, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(sh_frag_basic,
                           size_buffer_info,
                           NULL,
                           buffer_info);
        fprintf(stderr, "Compilation of fragment shader failed: %s\n",
                buffer_info);
    }

    /* Set up shader program. */
    GLuint shp_basic_shaders = glCreateProgram();
    glAttachShader(shp_basic_shaders, sh_vert_basic);
    glAttachShader(shp_basic_shaders, sh_frag_basic);
    /* Link all the attached shaders. */
    glLinkProgram(shp_basic_shaders);
    /* Check the link status. */
    glGetProgramiv(shp_basic_shaders, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shp_basic_shaders,
                            size_buffer_info,
                            NULL,
                            buffer_info);
        fprintf(stderr, "Failed to link shader: %s\n", buffer_info);
    }
    /* Delete shaders. */
    glDeleteShader(sh_vert_basic);
    glDeleteShader(sh_frag_basic);

    // ========================================
    // == Display loop
    // ========================================

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    while(!glfwWindowShouldClose(window)){

        glClear(GL_COLOR_BUFFER_BIT);

        glfwPollEvents();

        glUseProgram(shp_basic_shaders);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glUseProgram(0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }
}
