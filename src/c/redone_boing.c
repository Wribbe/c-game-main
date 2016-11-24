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

GLuint texture;

void display(VAO * vao, GLuint shader_program)
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
    const char * filename = data_src("test_rectangle_rainbow.txt");
    load_data(&point_data, NULL, filename);
    float * second_buffer = malloc(sizeof(float) * point_data.elements);
    load_data(&point_data, second_buffer, filename);

    // Define VAO and VBO for vao and vbo.
    VAO vao = {0};
    VBO vbo = {0};

    // Generate buffers.
    gen_buffers(1, &vbo, &point_data, GL_STATIC_DRAW);

    // Set render type for vbo.
    vbo.render_geometry = GL_TRIANGLES;

    // Set up attribute pointer information.
    Attrib_Pointer_Info attribs[] = {
        (Attrib_Pointer_Info){
            .index = 0,
            .size = 3,
            .offset = NULL,
        },
        (Attrib_Pointer_Info){
            .index = 1,
            .size = 3,
            .offset = (GLvoid * )(3*sizeof(GL_FLOAT)),
        },
        (Attrib_Pointer_Info){
            .index = 2,
            .size = 2,
            .offset = (GLvoid * )(6*sizeof(GL_FLOAT)),
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

    printf("elements: %d, rows: %d\n", point_data.elements, point_data.rows);
    printf("count: %d\n", vao.count);

//    // Load image test.
    size_t width = 0;
    size_t height = 0;
//
    unsigned char * jpeg_data;
//
//    unsigned char * jpeg_data = load_test(&height, &width);
    load_test(&jpeg_data, &height, &width);
    printf("width: %zu, height: %zu\n", width, height);
//
//    glGenTextures(1, &texture);
//
//    glBindTexture(GL_TEXTURE_2D, texture);
//
//    size_t size = width * height * 3;
//    printf("jpeg data: ");
//    for (size_t i = 0; i<size; i++) {
//        printf("%c", jpeg_data[i]);
//    }
//
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

//    int width, height;
//    filename = texture_src("green.jpg");
//    filename = "/home/steff/c/c-game-main/data/textures/Dietrich.jpg";
//    unsigned char * image = SOIL_load_image(filename, &width, &height, 0, SOIL_LOAD_RGB);
//    printf("SOIL: width: %d heigth: %d\n", width, height);
//    if (!image) {
//        printf("File could not be loaded: %s\n", filename);
//    }

//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, jpeg_data);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, fake_data);
//    glGenerateMipmap(GL_TEXTURE_2D);

//    free(jpeg_data);
    // Unbind the texture.
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    while(!glfwWindowShouldClose(window)) {

        display(&vao, shader_program);
        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    glfwTerminate();
    free(second_buffer);
}
