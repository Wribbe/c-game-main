#include <math.h>
#include <pthread.h>

#include "utils/utils.h"
#include "graphics/graphics.h"
#include "SOIL/SOIL.h"
#include "globals/globals.h"
#include "linmath.h"

int main(void) {

    const char * controllable_data = "test_rectangle_rainbow.txt";
    const char * window_data = "window_component_data.txt";

    // Initialize everything.
    GLFWwindow * window = window_init(1000, 1000, "Refactoring Playground.");

    // Create standard shader_program.
    GLuint shader_program = create_shader_program("boing.vert", "boing.frag");
    GLuint controllable_shader_program = create_shader_program("controllable.vert",
                                                               "controllable.frag");

    // Create vao based on point_data.
    VAO * vao = create_vao(controllable_data, GL_DYNAMIC_DRAW, GL_TRIANGLES);

    // Create window vao.
    VAO * window_vao = create_vao(window_data, GL_STATIC_DRAW, GL_TRIANGLES);

    // Generate texture and load image data.
    GLuint texture = create_texture("Dietrich.jpg");

    // Setup environment variables.
    setup_globals();

    // Enable alpha blending.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glfwSwapInterval(0);

    while(!glfwWindowShouldClose(window)) {
        frame_start();
        poll_events(window);
        // Clear screen.
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        frame_stop();
    }

    glfwTerminate();
    free_vao(vao);
    free_vao(window_vao);
}
