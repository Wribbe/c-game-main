#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "linmath.h"

void init(void)
{
    // Set background color.
    glClearColor( 0.55f, 0.55f, 0.55f, 0.0f);
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT);
}

typedef struct {
    GLFWwindow * window;
    void (* GLFWkeyfun)(GLFWwindow *, int, int, int, int);
    int * keymap;
} key_callback_data;

void callback_key(
                  GLFWwindow * window,
                  int key,
                  int scancode,
                  int action,
                  int mods
                 )
{
    printf("Key: %d\n", key);
}

void * setup_callback(void * data) {
    /* Worker function for thread for setting up key callback. */
    printf("Hello from thread.\n");
    return NULL;
}


int main(void)
{
    GLFWwindow * window;

    // Init GLFW.
    if (!glfwInit()) {
        return EXIT_FAILURE;
    }

    window = glfwCreateWindow(400, 400, "Recreated Boing", NULL, NULL);
    if (!window) {
        fprintf(stderr, "[!] Could not create window, aborting.\n");
        return EXIT_FAILURE;
    }

    glfwSetWindowAspectRatio(window, 1, 1);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    int key_map[512];
    key_callback_data callback_data = {
        window,
        callback_key,
        key_map,
    };

    pthread_t key_callback_thread;

    pthread_create(&key_callback_thread, NULL, setup_callback, &callback_data);

    // Set key callback function for window.
    glfwSetKeyCallback(window, callback_key);

    // Initialize values.
    init();

    while(!glfwWindowShouldClose(window)) {

        display();
        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    return EXIT_SUCCESS;
}
