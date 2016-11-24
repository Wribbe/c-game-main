#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "linmath.h"

#include "events/events.h"
#include "graphics/graphics.h"
#include "utils/utils.h"
#include "SOIL/SOIL.h"

void init(void)
{
    // Set background color.
    glClearColor( 0.55f, 0.55f, 0.55f, 0.0f);
}

void display(VAO * vao, GLuint shader_program, GLuint texture)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUseProgram(shader_program);
    glBindVertexArray(vao->vao);
    glDrawArrays(vao->vbo.render_geometry, vao->start, vao->count);
    glBindVertexArray(0);
}

int main(void)
{
    GLFWwindow * window;
    const char * filename;

    // Init GLFW.
    if (!glfwInit()) {
        return EXIT_FAILURE;
    }

    /* OpenGL window context hints. */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(1000, 1000, "Recreated Boing", NULL, NULL);
    if (!window) {
        fprintf(stderr, "[!] Could not create window, aborting.\n");
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    // Set key callback function for window.
    glfwSetKeyCallback(window, callback_key);

    // Initialize values.
    init();

    // Set up shaders.
    GLuint vertex_shader = 0;
    GLuint fragment_shader = 0;

    create_shader(&vertex_shader, shader_src("boing.vert"));
    create_shader(&fragment_shader, shader_src("boing.frag"));

    // Create and link program.
    GLuint shaders[] = {
        vertex_shader,
        fragment_shader,
    };

    // Set up and link shader program.
    GLuint shader_program = 0;
    link_program(&shader_program, shaders, SIZE(shaders));

    // Gather data.
    Point_Data point_data = {0};
    // Set up filename.
    filename = data_src("test_rectangle_rainbow.txt");
    // Run load vertex data without buffer to get dimensions.
    load_data(&point_data, NULL, filename);
    // Allocate memory for vertex data.
    float * vertex_buffer = malloc(sizeof(float) * point_data.elements);
    // Run load vertex with buffer to process the vertices.
    load_data(&point_data, vertex_buffer, filename);

    // Define VAO and VBO for vao and vbo.
    VAO vao = {0};
    VBO vbo = {0};

    // Generate buffers.
    gen_buffers(1, &vbo, &point_data, GL_STATIC_DRAW);

    // Set render type for vbo.
    vbo.render_geometry = GL_TRIANGLES;

    // Set up attribute pointer information.
    Attrib_Pointer_Info attribs[] = {
        (Attrib_Pointer_Info){ // Vertex data.
            .index = 0,
            .size = 3,
        },
        (Attrib_Pointer_Info){ // Color data.
            .index = 1,
            .size = 3,
        },
        (Attrib_Pointer_Info){ // Texture coord data.
            .index = 2,
            .size = 2,
        },
    };

    // Set up the vbos that should be bound to the vao.
    VBO * vbo_binds[] = {
        &vbo,
    };

    // Generate vertex array buffer.
    gen_vertex_arrays(
                      1,
                      &vao,
                      vbo_binds,
                      SIZE(vbo_binds),
                      attribs,
                      SIZE(attribs)
                     );

    // Set vbo as vao.vbo.
    vao.vbo = vbo;
    // Set start.
    vao.start = 0;
    // Set count.
    vao.count = vao.vbo.point_data->rows;

    // Get texture file source.
    filename = texture_src("Dietrich.jpg");

    // Generte texture and load image data.
    GLuint texture;
    glGenTextures(1, &texture);
    load_to_texture(&texture, filename);

    while(!glfwWindowShouldClose(window)) {

        display(&vao, shader_program, texture);
        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    glfwTerminate();
    free(vertex_buffer);
}
