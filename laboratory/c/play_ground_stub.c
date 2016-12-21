#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "linmath.h"

#include "events/events.h"
#include "graphics/graphics.h"
#include "utils/utils.h"
#include "SOIL/SOIL.h"
#include "components/components.h"
#include "globals/globals.h"

GLFWwindow * play_init(void)
{

    // Init GLFW.
    if (!glfwInit()) {
        fprintf(stderr, "Could not intialize GLFW, aborting.\n");
        exit(1);
    }

    /* OpenGL window context hints. */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow * window = glfwCreateWindow(1000, 1000, "Playground", NULL, NULL);
    if (!window) {
        fprintf(stderr, "[!] Could not create window, aborting.\n");
        exit(1);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    // Set key callback function for window.
    glfwSetKeyCallback(window, callback_key);

    // Initialize values.
    global_init();

    return window;
}

GLuint create_shader_program(
                             const char * source_vertex,
                             const char * source_fragment
                            )
{
    // Set up shaders.
    GLuint vertex_shader = 0;
    GLuint fragment_shader = 0;

    const char * vert_path = texture_src(source_vertex);
    const char * frag_path = texture_src(source_fragment);

    create_shader(&vertex_shader, vert_path);
    create_shader(&fragment_shader, frag_path);

    // Create and link program.
    GLuint shaders[] = {
        vertex_shader,
        fragment_shader,
    };

    // Set up and link shader program.
    GLuint shader_program = 0;
    link_program(&shader_program, shaders, SIZE(shaders));

    // Delete shaders.
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    // Free strings.
    free(vert_path);
    free(frag_path);

    // Return program GLuint.
    return shader_program;
}

int main(void) {

    const char * filename;

    // Initialize everything.
    GLFWwindow * window = play_init();

    // Create standard shader_program.
    GLuint shader_program = create_shader_program("boing.vert", "boing.frag");

    // Set up controllable shader variables.
    GLuint controllable_vertex = 0;
    GLuint controllable_fragment = 0;

    // Create shaders from text source.
    create_shader(&controllable_vertex, shader_src("controllable.vert"));
    create_shader(&controllable_fragment, shader_src("controllable.frag"));

    // Set up shader list.
    GLuint controllable_shaders[] = {
        controllable_vertex,
        controllable_fragment,
    };

    // Create and link a new shader program.
    GLuint controllable_shader_program = 0;
    link_program(&controllable_shader_program,
                 controllable_shaders,
                 SIZE(controllable_shaders));

    // Set up filename.
    filename = data_src("test_rectangle_rainbow.txt");
    // Load data to point data.
    Point_Data * point_data = load_data(filename);

    // Create vao based on point_data.
    VAO * vao = create_vao(point_data, GL_DYNAMIC_DRAW, GL_TRIANGLES);

    // Create window vao.
    filename = data_src("window_component_data.txt");
    Point_Data * window_points = load_data(filename);
    VAO * window_vao = create_vao(window_points, GL_STATIC_DRAW, GL_TRIANGLES);


    // Get texture file source.
    filename = texture_src("Dietrich.jpg");

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
    GLuint texture;
    glGenTextures(1, &texture);
    load_to_texture(&texture, filename);

    // Setup environment variables.
    setup_globals();

    // Enable alpha blending.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    struct timespec start_time = {0};
    struct timespec end_time = {0};

    printf("\n");

    glfwSwapInterval(0);

    while(!glfwWindowShouldClose(window)) {
        clock_gettime(CLOCK_REALTIME, &start_time);
        global_variables[glfw_time] = (float)glfwGetTime();
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
        clock_gettime(CLOCK_REALTIME, &end_time);
        double millis = 0;
        millis += (end_time.tv_sec - start_time.tv_sec) * 1000;
        millis += (end_time.tv_nsec - start_time.tv_nsec) / 1e6;
        float time = (float)millis / (1000.0f / 60.0f);
        set_timestep(time);
        printf("\rFrames per second: %f", 1000.0f / millis);
    }

    glfwTerminate();
    free_point_data(point_data);
    free_point_data(window_points);
    free_component(main_component);
    free_component(second_component);
    free_vao(vao);
    free_vao(window_vao);
}
