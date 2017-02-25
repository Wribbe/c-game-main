#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#include "glad.h"
#include <GLFW/glfw3.h>
#include "portaudio.h"

#define UNUSED(x) (void)x
#define SIZE(x) sizeof(x)/sizeof(x[0])

/* Global variables. */
#define WIDTH 800
#define HEIGHT 600
#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif

/* Global storage. */
double m_xpos = 0;
double m_ypos = 0;
#define KEY_SIZE GLFW_KEY_LAST
#define QUEUE_SIZE 1024
enum key_layers {
    TIME_PRESS,
    TIME_RELEASE,
    NUM_LAYERS,
};
double keymap[NUM_LAYERS][KEY_SIZE];
bool held_status[KEY_SIZE] = {false};
int command_queue[QUEUE_SIZE];
int * command_queue_end = command_queue;
/* Global states. */
struct state {
    GLFWwindow * window;
};
struct state STATE = {0};
struct mapping_node {
    int key;
    int modifier_sum;
    struct function_guard * function_guard;
    void * data;
    struct mapping_node * next;
};
struct mapping_node * command_bindings[KEY_SIZE] = {0};
struct function_guard  {
    bool atomic;
    bool been_run;
    void (*action_function)(int key, int action, void * data);
};


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

const char * get_key_name(int key)
    /* Name and return additional keys. */
{
    const char * key_name = glfwGetKeyName(key, 0);
    if (key_name != NULL) {
        return key_name;
    }
    switch(key) {
        case(GLFW_KEY_SPACE): return "space"; break;
        case(GLFW_KEY_ESCAPE): return "escape"; break;
        case(GLFW_KEY_ENTER): return "enter"; break;
        case(GLFW_KEY_TAB): return "tab"; break;
        case(GLFW_KEY_BACKSPACE): return "backspace"; break;
        case(GLFW_KEY_INSERT): return "insert"; break;
        case(GLFW_KEY_DELETE): return "delete"; break;
        case(GLFW_KEY_RIGHT): return "right"; break;
        case(GLFW_KEY_LEFT): return "left"; break;
        case(GLFW_KEY_DOWN): return "down"; break;
        case(GLFW_KEY_UP): return "up"; break;
        case(GLFW_KEY_PAGE_UP): return "page up"; break;
        case(GLFW_KEY_PAGE_DOWN): return "page down"; break;
        case(GLFW_KEY_HOME): return "home"; break;
        case(GLFW_KEY_END): return "end"; break;
        case(GLFW_KEY_CAPS_LOCK): return "caps lock"; break;
        case(GLFW_KEY_SCROLL_LOCK): return "scroll lock"; break;
        case(GLFW_KEY_NUM_LOCK): return "num lock"; break;
        case(GLFW_KEY_PRINT_SCREEN): return "print screen"; break;
        case(GLFW_KEY_PAUSE): return "pause"; break;
        case(GLFW_KEY_F1): return "F1"; break;
        case(GLFW_KEY_F2): return "F2"; break;
        case(GLFW_KEY_F3): return "F3"; break;
        case(GLFW_KEY_F4): return "F4"; break;
        case(GLFW_KEY_F5): return "F5"; break;
        case(GLFW_KEY_F6): return "F6"; break;
        case(GLFW_KEY_F7): return "F7"; break;
        case(GLFW_KEY_F8): return "F8"; break;
        case(GLFW_KEY_F9): return "F9"; break;
        case(GLFW_KEY_F10): return "F10"; break;
        case(GLFW_KEY_F11): return "F11"; break;
        case(GLFW_KEY_F12): return "F12"; break;
        case(GLFW_KEY_F13): return "F13"; break;
        case(GLFW_KEY_F14): return "F14"; break;
        case(GLFW_KEY_F15): return "F15"; break;
        case(GLFW_KEY_F16): return "F16"; break;
        case(GLFW_KEY_F17): return "F17"; break;
        case(GLFW_KEY_F18): return "F18"; break;
        case(GLFW_KEY_F19): return "F19"; break;
        case(GLFW_KEY_F20): return "F20"; break;
        case(GLFW_KEY_F21): return "F21"; break;
        case(GLFW_KEY_F22): return "F22"; break;
        case(GLFW_KEY_F23): return "F23"; break;
        case(GLFW_KEY_F24): return "F24"; break;
        case(GLFW_KEY_F25): return "F25"; break;
        case(GLFW_KEY_KP_0): return "Key-pad 0"; break;
        case(GLFW_KEY_KP_1): return "Key-pad 1"; break;
        case(GLFW_KEY_KP_2): return "Key-pad 2"; break;
        case(GLFW_KEY_KP_3): return "Key-pad 3"; break;
        case(GLFW_KEY_KP_4): return "Key-pad 4"; break;
        case(GLFW_KEY_KP_5): return "Key-pad 5"; break;
        case(GLFW_KEY_KP_6): return "Key-pad 6"; break;
        case(GLFW_KEY_KP_7): return "Key-pad 7"; break;
        case(GLFW_KEY_KP_8): return "Key-pad 8"; break;
        case(GLFW_KEY_KP_9): return "Key-pad 9"; break;
        case(GLFW_KEY_KP_DECIMAL): return "Key-pad decimal"; break;
        case(GLFW_KEY_KP_DIVIDE): return "Key-pad divide"; break;
        case(GLFW_KEY_KP_MULTIPLY): return "Key-pad multiply"; break;
        case(GLFW_KEY_KP_SUBTRACT): return "Key-pad subtract"; break;
        case(GLFW_KEY_KP_ADD): return "Key-pad add"; break;
        case(GLFW_KEY_KP_ENTER): return "Key-pad enter"; break;
        case(GLFW_KEY_KP_EQUAL): return "Key-pad equal"; break;
        case(GLFW_KEY_LEFT_SHIFT): return "left shift"; break;
        case(GLFW_KEY_LEFT_CONTROL): return "left control"; break;
        case(GLFW_KEY_LEFT_ALT): return "left alt"; break;
        case(GLFW_KEY_LEFT_SUPER): return "left super"; break;
        case(GLFW_KEY_RIGHT_SHIFT): return "right shift"; break;
        case(GLFW_KEY_RIGHT_CONTROL): return "right control"; break;
        case(GLFW_KEY_RIGHT_ALT): return "right alt"; break;
        case(GLFW_KEY_RIGHT_SUPER): return "right super"; break;
    }
    return key_name;
}

struct held_node {
    bool in_use;
    int key;
    int next;
};
#define HELD_NULL -1
int held_keys = HELD_NULL;
#define NUM_HELD 80
struct held_node held_arena[NUM_HELD];

bool press(int action); // Forward declare for later function.
bool release(int action); // Forward declare for later function.
void callback_simple_keyboard(GLFWwindow * window,
                              int key,
                              int scancode,
                              int action,
                              int mods)
    /* Simple callback function for keyboard input. */
{
    if (action == GLFW_REPEAT) {
        return;
    }
    double current_time = glfwGetTime();
    keymap[action][key] = current_time;
    UNUSED(mods);
    if (command_queue_end - command_queue == QUEUE_SIZE) {
        error_and_exit("Command queue full!");
    }
    *command_queue_end++ = key;
    *command_queue_end++ = action;
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

void callback_simple_scroll(GLFWwindow * window,
                            double xoffset,
                            double yoffset)
    /* Simple callback function for the mouse-wheel. */
{
    UNUSED(window);
    printf("Scroll, x: %f, y: %f\n", xoffset, yoffset);
}

bool press(int action)
{
    return action == GLFW_PRESS;
}

bool release(int action)
{
    return action == GLFW_RELEASE;
}

bool held(int key)
{
    return held_status[key];
}

double held_down_for(int key)
    /* Return the time a key was held down for. */
{
    double time = keymap[GLFW_RELEASE][key] - keymap[GLFW_PRESS][key];
    if (time < 0) {
        return 0;
    }
    return time;
}

int MOD_KEYS[] = {
    GLFW_KEY_RIGHT_CONTROL,
    GLFW_KEY_RIGHT_SHIFT,
    GLFW_KEY_RIGHT_ALT,
};
size_t NUM_MOD_KEYS = sizeof(MOD_KEYS)/sizeof(MOD_KEYS[0]);

void space(int key, int action, void * data)
{
    UNUSED(data);
    UNUSED(key);
    UNUSED(action);
    printf("SPAAAACE!\n");
}
struct function_guard g_space = {
    false,
    false,
    space,
};

void mod_space(int key, int action, void * data)
{
    UNUSED(data);
    UNUSED(key);
    UNUSED(action);
    printf("MOD SPAAAACE!\n");
}
struct function_guard g_mod_space = {
    false,
    false,
    mod_space,
};

void mod2_space(int key, int action, void * data)
{
    UNUSED(data);
    UNUSED(key);
    UNUSED(action);
    printf("MOD 2 SPAAAAAAAAAAAAAAAAAAAACE!\n");
}
struct function_guard g_mod2_space = {
    false,
    false,
    mod2_space,
};

void mod3_space(int key, int action, void * data)
{
    UNUSED(data);
    if (release(action)) {
        printf("MOD 3 SPAAAAAAAAAAAAAAAAAAAACE!\n");
        printf("Was held or %f seconds.\n", held_down_for(key));
    }
}
struct function_guard g_mod3_space = {
    false,
    false,
    mod3_space,
};

void close_window(int key, int action, void * data)
    /* Get reference to window address, close on release. */
{
    if (release(action)) {
        GLFWwindow ** window = (GLFWwindow **)data;
        glfwSetWindowShouldClose(*window, GLFW_TRUE);
    }
}
struct function_guard g_close_window = {
    false,
    false,
    close_window,
};

void play_sine(int key, int action, void * user_data);
struct function_guard g_play_sine = {
    true,
    false,
    play_sine,
};

int get_mod_sum(void) {
    int sum = 0;
    for (size_t i=0; i<NUM_MOD_KEYS; i++) {
        int key = MOD_KEYS[i];
        if (held_status[key]) {
            sum += key;
        }
    }
    return sum;
}

struct function_guard * guards[] = {
    &g_space,
    &g_mod_space,
    &g_mod2_space,
    &g_mod3_space,
    &g_close_window,
    &g_play_sine,
};

void event_action(int key, int action)
    /* React to events in event queue. */
{
    struct mapping_node * mapping = command_bindings[key];
    if (mapping != NULL) {
        struct mapping_node * pointer = mapping;
        int mod_sum = get_mod_sum();
        for(;pointer!=NULL;pointer = pointer->next) {
            if (pointer->modifier_sum == mod_sum) {
                struct function_guard * guard = pointer->function_guard;
                if (guard->atomic && guard->been_run && press(action)) {
                    return;
                }
                guard->been_run = press(action) ? true : false;
                guard->action_function(key, action, pointer->data);
                break;
            }
        }
    }
}

void process_events(void)
    /* Process the event queue. */
{
    if (held_keys != HELD_NULL) {
        int index = held_keys;
        for(;index != HELD_NULL; index=held_arena[index].next) {
            int key = held_arena[index].key;
            event_action(key, GLFW_PRESS);
        }
    }
    int * ptr_queue = command_queue;
    for (; ptr_queue != command_queue_end; ptr_queue += 2) {
        int key = *ptr_queue;
        int action = *(ptr_queue+1);
        if(held_status[key] && press(action)) {
            continue;
        }

        event_action(key, action);

        bool * key_held = &held_status[key];
        if(!(*key_held) && press(action)) {
            *key_held = true;
            struct held_node * current_node = NULL;
            size_t current_index = 0;
            for (size_t i=0; i<NUM_HELD; i++) {
                struct held_node * temp = &held_arena[i];
                if (!(temp->in_use)) {
                    current_node = temp;
                    current_node->in_use = true;
                    current_index = i;
                    break;
                }
            }
            if (current_node != NULL) {
                current_node->key = key;
                if (held_keys == HELD_NULL) {
                    current_node->next = HELD_NULL;
                } else {
                    current_node->next = held_keys;
                }
                held_keys = current_index;
            }
        } else if (*key_held && release(action)) {
            *key_held = false;
            struct held_node * pointer = NULL;
            if (held_keys != HELD_NULL) {
                pointer = &held_arena[held_keys];
            }
            struct held_node * prev = NULL;
            for(;pointer != NULL; prev=pointer,pointer=&held_arena[pointer->next]){
                if (pointer->key == key) {
                    if(prev == NULL) {
                        held_keys = pointer->next;
                    } else {
                        prev->next = pointer->next;
                    }
                    pointer->in_use = false;
                    break;
                }
            }
        }
    }
    command_queue_end = command_queue;
}

struct mapping_node * mappings;
size_t num_mappings = 0;

void setup(void)
    /* Do necessary setup. */
{
    struct mapping_node local_mappings[] = {
        {GLFW_KEY_SPACE, 0, &g_space, NULL, NULL},
        {GLFW_KEY_SPACE, MOD_KEYS[0]+MOD_KEYS[1]+MOD_KEYS[2], &g_mod3_space, NULL, NULL},
        {GLFW_KEY_SPACE, MOD_KEYS[0], &g_mod_space, NULL, NULL},
        {GLFW_KEY_SPACE, MOD_KEYS[0]+MOD_KEYS[1], &g_mod2_space, NULL, NULL},
        {GLFW_KEY_ESCAPE, 0, &g_close_window, (void *)&STATE.window, NULL},
        {GLFW_KEY_R, 0, &g_play_sine, NULL, NULL},
    };
    num_mappings = SIZE(local_mappings);
    mappings = malloc(num_mappings * sizeof(struct mapping_node));
    for (size_t i=0; i<num_mappings; i++) {
        mappings[i] = local_mappings[i];
        struct mapping_node * current = &mappings[i];
        struct mapping_node ** bound = &command_bindings[current->key];
        if (*bound == NULL) { // Empty list.
            *bound = current;
        } else { // Sort so that the largest mod-sum comes first.
            struct mapping_node * pointer = *bound;
            struct mapping_node * prev = NULL;
            for(;;prev=pointer,pointer=pointer->next) {
                if (current->modifier_sum > pointer->modifier_sum) {
                    if(prev == NULL) { // First in list.
                        current->next = pointer;
                        *bound = current;
                    } else { // Not first in list.
                        current->next = pointer;
                        prev->next = current;
                    }
                    break; // No more to be done.
                } else if (pointer->next == NULL) {  // No more elements.
                    pointer->next = current;
                    pointer->next->next = NULL;
                    break;
                }
            }
        }
    }
}

#define NUM_SECONDS   (5)
#define SAMPLE_RATE   (44100)
#define FRAMES_PER_BUFFER  (64)
#define TABLE_SIZE 200
struct paTestData {
    float sine[TABLE_SIZE];
    int left_phase;
    int right_phase;
    char message[20];
};

static int patestCallback(const void * inputBuffer,
                          void * outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo * timeinfo,
                          PaStreamCallbackFlags statusFlags,
                          void * userData)
{
    struct paTestData * data = (struct paTestData *)userData;
    float * out = (float *)outputBuffer;

    UNUSED(timeinfo);
    UNUSED(statusFlags);
    UNUSED(inputBuffer);

    for (size_t i=0; i<framesPerBuffer; i++) {
        *out++ = data->sine[data->left_phase];
        *out++ = data->sine[data->right_phase];
        data->left_phase += 1;
        if (data->left_phase >=TABLE_SIZE) {
            data->left_phase -= TABLE_SIZE;
        }
        data->right_phase += 3;
        if (data->right_phase >=TABLE_SIZE) {
            data->right_phase -= TABLE_SIZE;
        }
    }
    return paContinue;
}

pthread_cond_t sig_work = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock_work_queue = PTHREAD_MUTEX_INITIALIZER;

void play_sine(int key, int action, void * user_data)
    /* Use PortAudio to play a constructed sine wave. */
{
    pthread_mutex_lock(&lock_work_queue);
    pthread_cond_signal(&sig_work);
    pthread_mutex_unlock(&lock_work_queue);
    return;
    if (release(action)) {
        return;
    }
    printf("Inside play_sine.\n");
    /* Set up variables. */
    PaStreamParameters outputParameters = {0};
    PaStream * stream = NULL;
    PaError err = 0;
    struct paTestData data = {0};

    /* Create sine-data. */
    for (int i=0; i<TABLE_SIZE; i++) {
        data.sine[i] = (float)sin(((double)i/(double)TABLE_SIZE) * M_PI * 2.0f);
    }
    data.left_phase = 0;
    data.right_phase = 0;

    err = Pa_Initialize();
    if(err != paNoError) {
        error_and_exit("Could not initialize PortAudio.");
    }

    outputParameters.device = Pa_GetDefaultOutputDevice();
    if (outputParameters.device == paNoDevice) {
        error_and_exit("Could not find a default audio device.");
    }

    outputParameters.channelCount = 2; // Stereo.
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->\
                                        defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
                        &stream,
                        NULL, // No input.
                        &outputParameters,
                        SAMPLE_RATE,
                        FRAMES_PER_BUFFER,
                        paClipOff, // Not overshooting range, don't bother clipping.
                        patestCallback,
                        &data
                       );
    err = Pa_StartStream(stream);
    if (err != paNoError) {
        error_and_exit("Error when playing PA stream.");
    }
    printf("Play for %d seconds.\n", NUM_SECONDS);
    Pa_Sleep(NUM_SECONDS * 1000);
    err = Pa_StopStream(stream);
    if (err != paNoError) {
        error_and_exit("Error when stopping stream.");
    }
    err = Pa_CloseStream(stream);
    if (err != paNoError) {
        error_and_exit("Error when closing stream.");
    }
}

#define NUM_THREADS 10
pthread_t thread_pool[NUM_THREADS];
bool kill_threads = false;

void * thread_setup(void * data)
{
    printf("Waiting for lock.\n");
    pthread_mutex_lock(&lock_work_queue);
    for(;;) {
        printf("Waiting on sig_work.\n");
        pthread_cond_wait(&sig_work, &lock_work_queue);

        if (kill_threads) {
            return NULL;
        }
        printf("Got signal that there is something to do?\n");
    }
}

void setup_thread_pool(void)
    /* Set up thread pool. */
{
    for (size_t i=0; i<NUM_THREADS; i++) {
        pthread_create(&thread_pool[i], NULL, thread_setup, NULL);
    }
}

int main(int argc, char ** argv)
{
    if (!glfwInit()) {
        error_and_exit("Could not initialize GLFW, aborting.\n");
    }

    UNUSED(argc);
    setup();
    setup_thread_pool();

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
    STATE.window = window;

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    /* Set window callback function. */
    glfwSetKeyCallback(window, callback_simple_keyboard);
    glfwSetMouseButtonCallback(window, callback_simple_mouse);
    glfwSetCursorPosCallback(window, callback_simple_cursor_pos);
    glfwSetScrollCallback(window, callback_simple_scroll);

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
        process_events();

        glUseProgram(shp_basic_shaders);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glUseProgram(0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }
    glfwTerminate();
    free(mappings);
    Pa_Terminate();
}
