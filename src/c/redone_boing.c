#include <math.h>
#include <pthread.h>

#include "utils/utils.h"
#include "events/events.h"
#include "graphics/graphics.h"
#include "SOIL/SOIL.h"
#include "components/components.h"
#include "globals/globals.h"
#include "linmath.h"

int main(void) {

    const char * filename;

    // Initialize everything.
    GLFWwindow * window = window_init(1000, 1000, "Refactoring Playground.");

    // Create standard shader_program.
    GLuint shader_program = create_shader_program("boing.vert", "boing.frag");
    GLuint controllable_shader_program = create_shader_program("controllable.vert",
                                                               "controllable.frag");

    // Create vao based on point_data.
    VAO * vao = create_vao("test_rectangle_rainbow.txt",
                           GL_DYNAMIC_DRAW,
                           GL_TRIANGLES);

    // Create window vao.
    VAO * window_vao = create_vao("window_component_data.txt",
                                  GL_STATIC_DRAW,
                                  GL_TRIANGLES);


    // Set up main component.
    struct component * main_component = create_component("Dietrich_1",
                                                         vao,
                                                         NULL);

    // set up second component.
    struct component * second_component = create_component("Dietrich_2",
                                                           vao,
                                                           NULL);

    // set up third component.
    struct component * third_component = create_component("Dietrich_3",
                                                           vao,
                                                           NULL);

    // Scale the dimensions.
    scale_component(main_component, 0.4, 0.3, 0.3);
    scale_component(second_component, 0.4, 0.3, 0.3);
    scale_component(third_component, 0.1, 0.2, 0.2);

    // Append controllable to corrct list.
    append_component(main_component, CONTROLLABLE);
    append_component(second_component, CONTROLLABLE);
    append_component(third_component, CONTROLLABLE);

    // Create window component.
    struct component * window_component = create_component("window",
                                                           window_vao,
                                                           NULL);
    // Append to correct list.
    append_component(window_component, SCENE_COMPONENTS);
    // Add specific collision function to window.
    set_collision_function(window_component, collision_keep_inside_border);

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
        draw_components(get_component(SCENE_COMPONENTS),
                        shader_program,
                        texture);
        draw_components(get_component(CONTROLLABLE),
                        shader_program,
                        texture);
        draw_component(controlled_component,
                       shader_program,
                       texture);
        draw_component(controlled_component,
                       controllable_shader_program,
                       texture);
        glfwSwapBuffers(window);
        frame_stop();
    }

    glfwTerminate();
    free_component(main_component);
    free_component(second_component);
    free_vao(vao);
    free_vao(window_vao);
}
