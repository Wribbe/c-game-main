#include <stdlib.h>
#include <stdio.h>

#include "glad.h"
#include <GLFW/glfw3.h>


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
    "\n"
    "void main() {\n"
    "   gl_Position = vec4(position, 1.0f);\n"
    "}\n";


const GLchar * source_frag_basic = \
    "#version 330 core\n"
    "layout (location=1) in vec3 input_color;\n"
    "out vec4 color;\n"
    "\n"
    "void main() {\n"
    "   color = vec4(input_color, 1.0f);\n"
    "}\n";


GLfloat vertex_data_triangle[] = \
    {
        0.0f,  1.0f,  0.0f, 1.0f, 0.0f, 0.0f,
       -1.0f, -1.0f,  0.0f, 0.0f, 1.0f, 0.0f,
        1.0f, -1.0f,  0.0f, 0.0f, 0.0f, 1.0f,
    };



int main(void)
{
    if (!glfwInit()) {
        error_and_exit("Could not initialize GLFW, aborting.\n");
    }

    glfw_set_context();

    GLFWwindow * window = glfwCreateWindow(800,
                                           600,
                                           "windowing_basic",
                                           NULL,
                                           NULL);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

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
    size_t color_offset = 3;
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
