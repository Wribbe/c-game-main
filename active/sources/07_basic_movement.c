#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "glad.h"
#include <GLFW/glfw3.h>
#include "portaudio.h"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#include "vorbis/codec.h"
#include "vorbisfile.h"
#include "FLAC/stream_decoder.h"

#include <pthread.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <time.h>

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
    void * (*data_function)(void * parameters);
    void * parameters;
    struct mapping_node * next;
};
typedef void (*action_function_type)(int key, int action, void * data);
struct mapping_node * command_bindings[KEY_SIZE] = {0};
struct function_guard  {
    bool atomic;
    bool been_run;
    action_function_type action_function;
    void (*free)(void * data);
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
    "uniform mat4 transformation;\n"
    "\n"
    "out vec4 vertex_color;\n"
    "\n"
    "void main() {\n"
    "   gl_Position = transformation * vec4(position, 1.0f);\n"
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
        0.0f,  0.8f,  0.0f, 1.0f, 0.0f, 0.0f,
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
    UNUSED(window);
    UNUSED(scancode);
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

struct function_guard get_guard(action_function_type func)
{
    struct function_guard guard = {
        .atomic = false,
        .been_run = false,
        .action_function = func,
    };
    return guard;
}

struct function_guard g_space = {0};
struct function_guard g_mod_space = {0};
struct function_guard g_mod2_space = {0};
struct function_guard g_mod3_space = {0};
struct function_guard g_close_window = {0};
struct function_guard g_play_sound = {0};
struct function_guard g_move = {0};

void mod_space(int key, int action, void * data)
{
    UNUSED(data);
    UNUSED(key);
    UNUSED(action);
    printf("MOD SPAAAACE!\n");
}

void mod2_space(int key, int action, void * data)
{
    UNUSED(data);
    UNUSED(key);
    UNUSED(action);
    printf("MOD 2 SPAAAAAAAAAAAAAAAAAAAACE!\n");
}

void mod3_space(int key, int action, void * data)
{
    UNUSED(data);
    if (release(action)) {
        printf("MOD 3 SPAAAAAAAAAAAAAAAAAAAACE!\n");
        printf("Was held or %f seconds.\n", held_down_for(key));
    }
}

void close_window(int key, int action, void * data)
    /* Get reference to window address, close on release. */
{
    if (release(action)) {
        GLFWwindow ** window = (GLFWwindow **)data;
        glfwSetWindowShouldClose(*window, GLFW_TRUE);
    }
    UNUSED(key);
}

void move(int key, int action, void * data);

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
                void * data = pointer->data_function(pointer->parameters);
                guard->action_function(key, action, data);
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


enum PLAYBACK_TYPE {
    KEEP,       // Keep playing the original sound if it's playing.
    RESTART,    // If the sound is playing restart it from the beginning.
    OVERLAY,    // Play sounds on top of each other.
    STOP,       // Stop the current sound, don't play other one.
};

struct playback_info {
    char * name;
    enum PLAYBACK_TYPE type;
};

enum EFF_SOUND {
    VOICE_16_WAV,
};

struct queue_sound_node {
    uint16_t channels;
    int16_t * current;
    int16_t * end;
    enum PLAYBACK_TYPE type;
    struct sound_data * sound_data;
    bool pause;
    bool cancel;
    struct sound_instance * my_instance;
    struct queue_sound_node * next;
};

struct sound_instance {
    struct queue_sound_node * queue_node;
    struct sound_instance * next;
    struct sound_instance * prev;
};

struct sound_data {
    const char * name;
    size_t size;
    uint32_t rate;
    uint32_t channels;
    bool pause;
    bool cancel;
    uint16_t playing;
    int16_t * data;
    struct sound_instance * instances;
    void (*free)(void * data);
};

struct queue_sound_node * queue_sound = NULL;
struct queue_sound_node queue_sound_empty = {0};

bool sound_playback_type_handler(struct queue_sound_node * node)
{
    struct sound_instance * instances = node->sound_data->instances;
    switch(node->type) {
        case RESTART: // Cancel all other instances of this sound.
            for(;instances != NULL; instances=instances->next) {
                instances->queue_node->cancel = true;
            }
            // Unset own cancel attribute.
            node->cancel = false;
            node->sound_data->playing = true;
            break;
        case KEEP:
            if (node->sound_data->playing) {
                node->cancel = true;
            } else {
                node->sound_data->playing = true;
            }
            break;
        case STOP:
            for(;instances != NULL; instances=instances->next) {
                instances->queue_node->cancel = true;
            }
            node->sound_data->playing = false;
            break;
        case OVERLAY:
            node->sound_data->playing = true;
            break;
        default:
            error_and_exit("Unknown type in sound_playback_type_handler");
            break;
    }
    return true;
}

pthread_mutex_t mutex_queue_sound = PTHREAD_MUTEX_INITIALIZER;
void free_queue_sound_node(struct queue_sound_node * node);
void play_sound(int key, int action, void * data)
{
    UNUSED(key);
    pthread_mutex_lock(&mutex_queue_sound);
    if (release(action)) {
        struct queue_sound_node * node = NULL;
        node = (struct queue_sound_node *)data;
        free_queue_sound_node(node);
        pthread_mutex_unlock(&mutex_queue_sound);
        return;
    }
    struct queue_sound_node * node = (struct queue_sound_node *)data;
    if (sound_playback_type_handler(node)) {
        if (queue_sound->current == NULL) { // Queue is empty.
            queue_sound = node;
            node->next = node;
        } else { // Nodes present in queue.
            struct queue_sound_node * current_next = queue_sound->next;
            queue_sound->next = node;
            node->next = current_next;
        }
    }
    pthread_mutex_unlock(&mutex_queue_sound);
}

void print_sound_guard(int key, int action, void * data);
void * get_queue_sound_node(void * parameters);
void * pack_params(enum EFF_SOUND sound, enum PLAYBACK_TYPE);

struct params_get_queue_sound {
    enum EFF_SOUND sound;
    enum PLAYBACK_TYPE type;
};

enum sound_format {
    VORBIS,
    WAV,
    FLAC,
};


const char * get_filetype(const char * filename)
    /* Return string containing file ending, including the dot. */
{
    size_t len = strlen(filename);
    const char * char_pointer = filename+len-1;
    for(;*char_pointer != '.'; char_pointer--) {
        if (char_pointer == filename) {
            return filename;
        }
    }
    return char_pointer;
}

void flac_m_callback(const FLAC__StreamDecoder * decoder,
                     const FLAC__StreamMetadata * metadata,
                     void * client_data)
{
    UNUSED(decoder);

    struct sound_data * data = (struct sound_data *)client_data;

    size_t num_samples = metadata->data.stream_info.total_samples;
    size_t rate = metadata->data.stream_info.sample_rate;
    size_t channels = metadata->data.stream_info.channels;
    size_t bps = metadata->data.stream_info.bits_per_sample;

    data->channels = channels;
    data->rate = rate;
    data->size = num_samples*channels*(bps/8);
}

void flac_err_callback(const FLAC__StreamDecoder * decoder,
                       FLAC__StreamDecoderErrorStatus status,
                       void * client_data)
{
    UNUSED(status);
    UNUSED(decoder);
    UNUSED(client_data);
    error_and_exit("FLAC got error on callback.\n");
}


FLAC__int16 * flac_write_loc = NULL;
FLAC__StreamDecoderWriteStatus
flac_w_callback(const FLAC__StreamDecoder * decoder,
                const FLAC__Frame * frame,
                const FLAC__int32 * const buffer[],
                void * client_data)
{
    UNUSED(decoder);

    struct sound_data * data = (struct sound_data *)client_data;

    for(size_t i=0; i<frame->header.blocksize; i++) {
        for (size_t chan=0; chan<data->channels; chan++) {
            *flac_write_loc++ = (FLAC__int16)buffer[chan][i];
        }
    }

    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void read_sound(struct sound_data * data,
                enum sound_format type,
                const char * filepath)
    /* Read the sound from disk to sound_data struct.
     * If no data was read, set data pointer to NULL. */
{
    OggVorbis_File vf = {0};
    switch(type) {
        case WAV:
            data->data = (int16_t*)drwav_open_and_read_file_s16(filepath,
                                                                &data->channels,
                                                                &data->rate,
                                                                &data->size);
            data->free = free;
            break;
        case VORBIS:
            ov_fopen(filepath, &vf);
            vorbis_info * vi = ov_info(&vf, -1);
            data->rate = vi->rate;
            data->channels = vi->channels;
            int little_endian = 0;
            int sample_16_bit = 2;
            int sample_signed = 1;
            data->size = (long)ov_pcm_total(&vf, -1)*sample_16_bit*vi->channels;
            data->data = calloc(data->size, sizeof(int16_t));
            if (data->data == NULL) {
                error_and_exit("Could not allocate enough memory for VORBIS data.\n");
            }
            char buffer[4095];
            int current_section = 0;
            int eof = 0;
            long ret = 1;
            int16_t * buffer_pointer = data->data;
            while(!(eof = (ret == 0))) {
                ret = ov_read(&vf,
                              buffer,
                              sizeof(buffer),
                              little_endian,
                              sample_16_bit,
                              sample_signed,
                              &current_section);
                memcpy(buffer_pointer, buffer, ret);
                buffer_pointer += ret/sample_16_bit;
            }
            ov_clear(&vf);
            data->free = free;
            break;
        case FLAC:;
            FLAC__bool ok = true;
            FLAC__StreamDecoder * decoder = FLAC__stream_decoder_new();
            FLAC__StreamDecoderInitStatus init_status;
            init_status = FLAC__stream_decoder_init_file(decoder,
                                                         filepath,
                                                         flac_w_callback,
                                                         flac_m_callback,
                                                         flac_err_callback,
                                                         data);
            if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
                error_and_exit("Could not initialize FLAC decoder.");
            }
            ok = FLAC__stream_decoder_process_until_end_of_metadata(decoder);
            fprintf(stderr, "Decoding of FLACK file: %s: %s.\n", filepath,
                    ok? "succeeded" : "FAILED");
            data->data = calloc(data->size, sizeof(int16_t));
            if (data->data == NULL) {
                error_and_exit("Could not allocate enough data for FLAC file.\n");
            }
            flac_write_loc = data->data;
            ok = FLAC__stream_decoder_process_until_end_of_stream(decoder);
            FLAC__stream_decoder_delete(decoder);
            data->free = free;
            break;
        default:
            fprintf(stderr, "Got wrong sound_type in read_sound function.\n");
            break;
    }
}

struct sound_data load_sound(const char * filepath)
{
    struct sound_data data = {0};
    const char * filetype =  get_filetype(filepath);
    if (strcmp(filetype, ".ogg") == 0) {
        printf("Found ogg file.\n");
        read_sound(&data, VORBIS, filepath);
    } else if (strcmp(filetype, ".wav") == 0) {
        printf("Found wav file.\n");
        read_sound(&data, WAV, filepath);
    } else if (strcmp(filetype, ".flac") == 0) {
        printf("Found flac file.\n");
        read_sound(&data, FLAC, filepath);
    } else {
        fprintf(stderr, "Non supported file extension in file: %s\n", filepath);
    }
    return data;
}

void print_guard_arguments(int key, int action, void * data)
    /* Dummy method for testing function guards. */
{
    printf("Got key: %d\n", key);
    printf("Got action: %d\n", action);
    printf("Got data: %s\n", (const char *)data);
}

void print_sound_guard(int key, int action, void * data)
    /* Dummy method for testing function guards. */
{
    printf("Got key: %d\n", key);
    printf("Got action: %d\n", action);
    struct playback_info * info = (struct playback_info *)data;
    printf("Got name: %s\n", info->name);
    printf("Got PLAYBACK_TYPE: %d\n", info->type);
}

int16_t get_next_sound_value(struct queue_sound_node * node)
{
    int16_t value = *node->current;
    if (node->current <= node->end) {
        node->current++;
    }
    return value;
}

int16_t get_cutoff_sum(int64_t sum)
{
    if (sum == 0) {
        return 0;
    } else if (sum >= 0xffff/2) {
        return 0xffff/2;
    } else if (sum <= -(0xffff/2)) {
        return -0xffff/2;
    }
    return (int16_t)sum;
}


struct sound_instance * unlink_instance(struct queue_sound_node * node)
{
    // Unlink our instance.
    struct sound_instance * my_instance = node->my_instance;
    if (my_instance->prev == NULL) { // First in instance list.
        // Next can be NULL, which we want if it's the end.
        node->sound_data->instances = my_instance->next;
        if (my_instance->next != NULL) {
            my_instance->next->prev = NULL;
        }
    } else { // Not first in list.
        if (my_instance->next == NULL) { // Last in list.
            my_instance->prev->next = NULL;
        } else { // All other variants.
            my_instance->prev->next = my_instance->next;
            my_instance->next->prev = my_instance->prev;
        }
    }
    return my_instance;
}

void * thread_work_clean_queue_sound(void * data)
{
    UNUSED(data);
    pthread_mutex_lock(&mutex_queue_sound);

    pthread_t ** thread_pointer = (pthread_t **)data;

    if (queue_sound == &queue_sound_empty) { // Empty queue.
        pthread_mutex_unlock(&mutex_queue_sound);
        return NULL;
    }
    if (*thread_pointer == NULL) { // Should only be one active thread.
        pthread_mutex_unlock(&mutex_queue_sound);
        return NULL;
    }

    struct queue_sound_node * prev = queue_sound;
    struct queue_sound_node * node = queue_sound->next;
    struct queue_sound_node * temp = NULL;

    for(;;) {
        if (node == queue_sound) {
            break;
        }
        if (node->current >= node->end) { // Unlink and free.

            temp = node;
            node = node->next;
            prev->next = node;

            free_queue_sound_node(temp);
        } else { // Don't free, continue iteration.
            prev = node;
            node = node->next;
        }
    }
    // Node is pointing at the last node at this point.
    if (queue_sound->current >= queue_sound->end) {
        if (queue_sound == queue_sound->next) {
            // There is only one element in the queue.
            temp = queue_sound;
            queue_sound = &queue_sound_empty;
        } else {
            // Were more elements in the queue.
            temp = queue_sound;
            // Unlink the first node.
            queue_sound = queue_sound->next;
            // Keep the circular queue by relinking the last node.
            prev->next = queue_sound;
        }
        // Unlink our instance.
        free_queue_sound_node(temp);
    }
    // Reset pointer to mark that we're done.
    *thread_pointer = NULL;
    pthread_mutex_unlock(&mutex_queue_sound);
    return NULL;
}


pthread_attr_t cleanup_attribs = {0};
pthread_t thread_queue_sound_cleaner = {0};
pthread_t * thread_pointer_cleaner = NULL;

static int callback_pa(const void * input_buffer,
                       void * output_buffer,
                       unsigned long frames_per_buffer,
                       const PaStreamCallbackTimeInfo * timeinfo,
                       PaStreamCallbackFlags status_flags,
                       void * user_data)
    /* Main callback function for playing audio. */
{
    UNUSED(timeinfo);
    UNUSED(status_flags);
    UNUSED(input_buffer);

    pthread_mutex_lock(&mutex_queue_sound);
    struct queue_sound_node ** node_ptr = (struct queue_sound_node **)user_data;
    int16_t * out = (int16_t *)output_buffer;
    struct queue_sound_node * node = *node_ptr;
    bool someone_is_done = false;

    for (size_t i=0; i<frames_per_buffer; i++) {
        int64_t sum_left = 0;
        int64_t sum_right = 0;
        if (node->current != NULL) {
            struct queue_sound_node * pointer = node->next;
            for (;pointer != node; pointer = pointer->next) {
                if (pointer->cancel) {
                    // Set current == end, will end the sound.
                    pointer->current = pointer->end+1;
                } else if (pointer->pause) {
                    continue; // Pause playback of this sound.
                }
                if (pointer->current <= pointer->end) {
                    if (pointer->channels == 2) { // Interleaved sound for each channel.
                        sum_left += get_next_sound_value(pointer);
                        sum_right += get_next_sound_value(pointer);
                    } else { // Mono sound on both channels.
                        int16_t div = get_next_sound_value(pointer);
                        sum_left += div;
                        sum_right += div;
                    }
                } else {
                    someone_is_done = true;
                }
            }
            if (node->cancel) {
                // Set current == end, will end the sound.
                node->current = node->end+1;
            } else if (node->pause) {
                continue; // Pause playback of this sound.
            }
            if (node->current <= node->end) {
                if (node->channels == 2) {
                    sum_left += get_next_sound_value(node);
                    sum_right += get_next_sound_value(node);
                } else {
                    int16_t div = get_next_sound_value(node);
                    sum_left += div;
                    sum_right += div;
                }
            } else {
                someone_is_done = true;
            }
        }
        *out++ = get_cutoff_sum(sum_left);
        *out++ = get_cutoff_sum(sum_right);
    }
    if (someone_is_done && thread_pointer_cleaner == NULL) {
        // Spin up a cleaning thread.
        thread_pointer_cleaner = &thread_queue_sound_cleaner;
        pthread_create(thread_pointer_cleaner,
                       &cleanup_attribs,
                       thread_work_clean_queue_sound,
                       &thread_pointer_cleaner);
    }
    pthread_mutex_unlock(&mutex_queue_sound);
    return paContinue;
}


struct sound_data sounds[50] = {0};

struct sound_data * get_sound_data(enum EFF_SOUND sound)
{
    return &sounds[sound];
}

struct free_param_node {
    void * data;
    void (*free)(void * data);
    struct free_param_node * next;
};

struct free_param_node * queue_params = NULL;

void * pack_params(enum EFF_SOUND sound, enum PLAYBACK_TYPE type)
{
    struct params_get_queue_sound * params = NULL;
    params = calloc(1, sizeof(struct params_get_queue_sound));

    params->sound = sound;
    params->type = type;

    struct free_param_node * free_node = NULL;
    free_node = calloc(1, sizeof(struct free_param_node));
    free_node->data = params;
    free_node->free = free;
    free_node->next = NULL;
    if(queue_params != NULL) {
        free_node->next = queue_params;
    }
    queue_params = free_node;

    return params;
}

void free_queue_sound_node(struct queue_sound_node * node)
{
    struct sound_instance * my_instance = unlink_instance(node);
    free(my_instance);
    free(node);
}


void * get_queue_sound_node(void * parameters)
{
    struct params_get_queue_sound * params = NULL;
    params = (struct params_get_queue_sound *)parameters;

    struct queue_sound_node * node = calloc(1, sizeof(struct queue_sound_node));
    struct sound_data * sound_data = get_sound_data(params->sound);

    struct sound_instance * instance = NULL;
    instance = calloc(1,sizeof(struct sound_instance));
    node->my_instance = instance;

    instance->queue_node = node;
    instance->next = NULL;
    instance->prev = NULL;

    if (sound_data->instances != NULL) { // Not first element.
        instance->next = sound_data->instances;
        instance->next->prev = instance;
    }
    sound_data->instances = instance;

    node->channels = sound_data->channels;
    node->current = sound_data->data;
    node->end = sound_data->data+sound_data->size-1;
    node->type = params->type;
    node->next = NULL;
    node->sound_data = sound_data;
    node->pause = false;
    node->cancel = false;

    return node;
}

void * pass_pointer(void * pointer)
{
    return pointer;
}

PaStreamParameters params_pa = {0};
PaStream * stream_pa = NULL;
PaStreamParameters default_pa_params(size_t channels);

/* Freetype related globals. */
FT_Library ft = {0};
FT_Face face = {0};
GLuint vao_fake = 0;

/* Source for text render shaders. */
const GLchar * source_vert_text = \
    "#version 330 core\n"
    "layout (location=0) in vec4 coord;\n"
    "out vec2 texcoord;\n"
    "\n"
    "void main(void) {\n"
    "   gl_Position = vec4(coord.xyz, 1.0f);\n"
    "   texcoord = coord.zw;\n"
    "}\n";

const GLchar * source_frag_text = \
    "#version 330 core\n"
    "in vec2 texcoord;\n"
    "uniform sampler2D tex;\n"
    "uniform vec4 uniform_color;\n"
    "out vec4 color;\n"
    "\n"
    "void main(void) {\n"
    "   color = vec4(1, 1, 1, texture2D(tex, texcoord).r)*uniform_color;\n"
    "}\n";


GLuint tex;
GLint uniform_text_sampler = 0;
GLint uniform_color = 0;
GLint uniform_tranformation = 0;
GLuint vbo_text = 0;

void create_texture_atlas(void);

#define KEY_UP GLFW_KEY_UP
#define KEY_DOWN GLFW_KEY_DOWN
#define KEY_LEFT GLFW_KEY_LEFT
#define KEY_RIGHT GLFW_KEY_RIGHT

void setup(void)
    /* Do necessary setup. */
{
    /* Set-up sound queue cleanup attributes. */
    pthread_attr_init(&cleanup_attribs);
    pthread_attr_setdetachstate(&cleanup_attribs, PTHREAD_CREATE_DETACHED);

    /* Initialize PortAudio. */
    if (Pa_Initialize() != paNoError) {
        error_and_exit("Could not initialize PortAudio, aborting.\n");
    }
    /* Set up stream. */
    params_pa = default_pa_params(2);
    queue_sound_empty.next = &queue_sound_empty;
    queue_sound_empty.current = NULL;
    queue_sound = &queue_sound_empty;
    Pa_OpenStream(&stream_pa,
                  NULL,
                  &params_pa,
                  44100,
                  128,
                  paClipOff,
                  callback_pa,
                  &queue_sound);
    Pa_StartStream(stream_pa);

    /* Load sounds. */
    const char * path = "input/voice_16.wav";
    //const char * path = "input/sine_16.wav";
    sounds[VOICE_16_WAV] = load_sound(path);
    if (sounds[VOICE_16_WAV].data == NULL) {
        printf("Got no data from: %s\n", path);
    } else {
        printf("Got data from: %s\n", path);
    }

    /* Setup key-bindings. */
    struct mapping_node local_mappings[] = {
        {GLFW_KEY_SPACE, 0, &g_space, pass_pointer, &g_space, NULL},
        {GLFW_KEY_SPACE, MOD_KEYS[0]+MOD_KEYS[1]+MOD_KEYS[2], &g_mod3_space, pass_pointer, &g_mod_space, NULL},
        {GLFW_KEY_SPACE, MOD_KEYS[0], &g_mod_space, pass_pointer, &g_mod_space, NULL},
        {GLFW_KEY_SPACE, MOD_KEYS[0]+MOD_KEYS[1], &g_mod2_space, pass_pointer, &g_mod2_space, NULL},
        {GLFW_KEY_ESCAPE, 0, &g_close_window, pass_pointer, &STATE.window, NULL},
        {GLFW_KEY_R, 0, &g_play_sound, get_queue_sound_node, pack_params(VOICE_16_WAV, RESTART), NULL},
        {GLFW_KEY_O, 0, &g_play_sound, get_queue_sound_node, pack_params(VOICE_16_WAV, OVERLAY), NULL},
        {GLFW_KEY_S, 0, &g_play_sound, get_queue_sound_node, pack_params(VOICE_16_WAV, STOP), NULL},
        {GLFW_KEY_K, 0, &g_play_sound, get_queue_sound_node, pack_params(VOICE_16_WAV, KEEP), NULL},
        {KEY_LEFT, 0, &g_move, pass_pointer, &g_move, NULL},
        {KEY_RIGHT, 0, &g_move, pass_pointer, &g_move, NULL},
        {KEY_UP, 0, &g_move, pass_pointer, &g_move, NULL},
        {KEY_DOWN, 0, &g_move, pass_pointer, &g_move, NULL},
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

    /* Setup keybinding function guards. */
    g_space = get_guard(space);
    g_mod_space = get_guard(mod_space);
    g_mod2_space = get_guard(mod2_space);
    g_mod3_space = get_guard(mod3_space);
    g_close_window = get_guard(close_window);
    g_play_sound = get_guard(play_sound);
    g_play_sound.atomic = true;
    g_move = get_guard(move);

    /* Set up freetype library. */
    if (FT_Init_FreeType(&ft)) {
        error_and_exit("Could not initialize FreeType library.");
    }

    /* Load FreeSans font. */
    if(FT_New_Face(ft, "fonts/FreeSans/FreeSans.ttf", 0, &face)) {
        error_and_exit("Could not load FreeSans font.");
    }

    /* Set font size. */
    FT_Set_Pixel_Sizes(face, 0, 48);
}

PaStreamParameters default_pa_params(size_t channels)
    /* Set and return struct for device parameters. */
{
    PaStreamParameters params = {0};

    params.device = Pa_GetDefaultOutputDevice();
    if (params.device == paNoDevice) {
        error_and_exit("Could not establish default device, aborting.\n");
    }
    params.channelCount = channels;
    params.sampleFormat = paInt16;
    params.suggestedLatency = Pa_GetDeviceInfo(params.device)->\
                              defaultLowOutputLatency;
    params.hostApiSpecificStreamInfo = NULL;
    return params;
}

#define START_CHARS 32
#define END_CHARS 128
#define NUM_CHARS END_CHARS - START_CHARS
int atlas_width = 0;
int atlas_height = 0;
struct info_character {
    float advance_x;
    float advance_y;
    float bitmap_width;
    float bitmap_rows;
    float bitmap_left;
    float bitmap_top;
    float offset_texture_x;
    float offset_texture_y;
} info_chars[NUM_CHARS];

int imax(int a, int b)
{
    if (a >= b) {
        return a;
    }
    return b;
}

void create_texture_atlas(void)
{
    int width = 0;
    int height = 0;
    /* Initial loading of charaders. */
    for (int i = START_CHARS; i<END_CHARS; i++) {
        if(FT_Load_Char(face, i, FT_LOAD_RENDER)) {
            fprintf(stderr, "Could not load character %c!\n", i);
            continue;
        }
        info_chars[i-START_CHARS] = (struct info_character){
            .advance_x = face->glyph->advance.x >> 6,
            .advance_y = face->glyph->advance.y >> 6,
            .bitmap_width = face->glyph->bitmap.width,
            .bitmap_rows = face->glyph->bitmap.rows,
            .bitmap_left = face->glyph->bitmap_left,
            .bitmap_top = face->glyph->bitmap_top,
            .offset_texture_x = 0,
            .offset_texture_y = 0,
        };
        width += face->glyph->bitmap.width;
        int rows = face->glyph->bitmap.rows;
        height = height > rows ? height : rows;
    }

    /* Set up single texture object to contain all glyphs. */
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        width,
        height,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        0
    );

    int x_offset = 0;
    int y_offset = 0;
    /* Generate textures and add to atlas, add offset to struct. */
    for (int i = 0; i<NUM_CHARS; i++) {
        if (FT_Load_Char(face, i+START_CHARS, FT_LOAD_RENDER)) {
            fprintf(stderr, "Could not load character %c!\n", i);
            continue;
        }
        info_chars[i].offset_texture_x = x_offset / (float)width;
        info_chars[i].offset_texture_y = y_offset / (float)height;
        float height = info_chars[i].bitmap_rows;
        float width = info_chars[i].bitmap_width;
        void * data = face->glyph->bitmap.buffer;
        glTexSubImage2D(GL_TEXTURE_2D,
                        0,
                        x_offset,
                        y_offset,
                        width,
                        height,
                        GL_RED,
                        GL_UNSIGNED_BYTE,
                        data);
        /* No y-offset at the moment, add to x-offset. */
        x_offset += width+1; // Why do we add 1?
    }
    atlas_width = width;
    atlas_height = height;
    glBindTexture(GL_TEXTURE_2D, 0);
}

typedef struct v4 {
    GLfloat x;
    GLfloat y;
    GLfloat z;
    GLfloat w;
} v4;

void render_text(const char * text,
                 float x,
                 float y,
                 GLuint program)
{
    glBindVertexArray(vao_fake);
    glUseProgram(program);

    float sx = 2.0 / WIDTH;
    float sy = 2.0 / HEIGHT;

    /* Set uniforms. */
    glUniform1i(uniform_text_sampler, 0); // Activated texture 0.
    GLfloat red[] = {1.0f, 0.0f, 0.0f, 1.0f};
    glUniform4fv(uniform_color, 1, red);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_text);
    GLuint attribute_coord = 0;
    glEnableVertexAttribArray(attribute_coord);

    size_t num_points_per_square = 6;
    size_t text_length = strlen(text);
    struct v4 coordinates[text_length*num_points_per_square];

    const char * pointer = text;
    struct v4 * coord_pointer = coordinates;
    for (; *pointer != '\0'; pointer++) {

        int char_index = *pointer - START_CHARS;
        struct info_character * info = &info_chars[char_index];

        float x2 = x + info->bitmap_left * sx;
        float y2 = y - info->bitmap_top * sy;
        float w = info->bitmap_width * sx;
        float h = info->bitmap_rows * sy;

        float proportional_width = info->bitmap_width/atlas_width;
        float proportional_height = info->bitmap_rows/atlas_height;

        float tex_u0 = info->offset_texture_x;
        float tex_u1 = tex_u0+proportional_width;
        float tex_v0 = info->offset_texture_y;
        float tex_v1 = tex_v0+proportional_height;

        struct v4 square[] = {
            // First triangle.
            {x2,    -y2,      tex_u0, tex_v0},
            {x2 + w,-y2,      tex_u1, tex_v0},
            {x2,    -y2 - h , tex_u0, tex_v1},
            // Second triangle.
            {x2 + w,-y2,      tex_u1, tex_v0},
            {x2,    -y2 - h , tex_u0, tex_v1},
            {x2 + w,-y2 - h , tex_u1, tex_v1},
        };
        memcpy(coord_pointer, &square, sizeof(square));
        coord_pointer += sizeof(square)/sizeof(square[0]);

        /* Advance cursor to start of next character. */
        x += info->advance_x * sx;
        y += info->advance_y * sy;
    }
    glBindTexture(GL_TEXTURE_2D, tex);

    glBufferData(GL_ARRAY_BUFFER, sizeof(coordinates), coordinates, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(attribute_coord, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_TRIANGLES, 0, sizeof(coordinates)/sizeof(coordinates[0]));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

typedef v4 m4[4];

/* Set up transformation matrix. */
m4 m4_transformation = {
    {1.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 1.0f},
};

static inline GLfloat * ptr_m4(v4 * vector)
{
    return &vector->x;
}

static inline GLfloat * translate(m4 * matrix, char coord)
{
    switch(coord){
        case 'x':
            return &matrix[0][0].w;
            break;
        case 'y':
            return &matrix[0][1].w;
            break;
        case 'z':
            return &matrix[0][2].w;
            break;
        default:
            error_and_exit("Undefined coord in translate.\n");
            return 0;
    }
}

float speed_movement = 0.01f;
void move(int key, int action, void * data)
{
    UNUSED(data);
    if (release(action)) {
        return;
    }
    *translate(&m4_transformation, 'x') += key==KEY_RIGHT ? speed_movement : 0;
    *translate(&m4_transformation, 'x') -= key==KEY_LEFT ? speed_movement : 0;
    *translate(&m4_transformation, 'y') -= key==KEY_DOWN ? speed_movement : 0;
    *translate(&m4_transformation, 'y') += key==KEY_UP ? speed_movement : 0;
}

void display_number(uint32_t number, float x, float y, GLuint program)
{
    char buffer[128] = {0};
    char * current = &buffer[126];
    while (number > 0) {
        int lowest = number % 10;
        number /= 10;
        *current-- = '0'+lowest;
        if (current <= buffer) {
            error_and_exit("Overflow in display number.");
        }
    }
    size_t number_size = &buffer[126] - current;
    render_text(current+1, x-((float)number_size * 0.1f), y, program);
}

int main(int argc, char ** argv)
{
    if (!glfwInit()) {
        error_and_exit("Could not initialize GLFW, aborting.\n");
    }

    ov_callbacks def = OV_CALLBACKS_DEFAULT;
    ov_callbacks no_close = OV_CALLBACKS_NOCLOSE;
    ov_callbacks stream = OV_CALLBACKS_STREAMONLY;
    ov_callbacks stream_no = OV_CALLBACKS_STREAMONLY_NOCLOSE;
    UNUSED(def);
    UNUSED(no_close);
    UNUSED(stream);
    UNUSED(stream_no);

    UNUSED(argc);
    setup();

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
    /* Generate fake VAO. */
    glGenVertexArrays(1, &vao_fake);

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

    /* Create shaders for text rendering. */
    GLuint sh_vert_text = glCreateShader(GL_VERTEX_SHADER);
    GLuint sh_frag_text = glCreateShader(GL_FRAGMENT_SHADER);
    /* Bind shader sources. */
    glShaderSource(sh_vert_text, 1, &source_vert_text, 0);
    glShaderSource(sh_frag_text, 1, &source_frag_text, 0);
    /* Use information space from above. */
    glCompileShader(sh_vert_text);
    glGetShaderiv(sh_vert_text, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(sh_vert_text,
                           size_buffer_info,
                           NULL,
                           buffer_info);
        fprintf(stderr, "Compilation of vertex shader failed: %s\n",
                buffer_info);
    }
    glCompileShader(sh_frag_text);
    glGetShaderiv(sh_frag_text, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(sh_frag_text,
                           size_buffer_info,
                           NULL,
                           buffer_info);
        fprintf(stderr, "Compilation of framgent shader failed: %s\n",
                buffer_info);
    }
    /* Set up shader program for text-rendering. */
    GLuint shp_text_shaders = glCreateProgram();
    glAttachShader(shp_text_shaders, sh_vert_text);
    glAttachShader(shp_text_shaders, sh_frag_text);
    /* Link all the attached shaders. */
    glLinkProgram(shp_text_shaders);
    /* Check link status. */
    glGetProgramiv(shp_text_shaders, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shp_text_shaders,
                            size_buffer_info,
                            NULL,
                            buffer_info);
        fprintf(stderr, "Failed to link shader %s\n", buffer_info);
    }
    /* Delete linked shaders. */
    glDeleteShader(sh_vert_text);
    glDeleteShader(sh_frag_text);

    /* Activate and generate texture 0. */
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &tex);

    glBindTexture(GL_TEXTURE_2D, tex);

    /* Set text rendering blending options. */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* Clamp texture to avoid artifacts. */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /* Disable 4-byte alignment. */
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glBindTexture(GL_TEXTURE_2D, 0);

    /* Set global uniform position values. */
    uniform_text_sampler = glGetUniformLocation(shp_text_shaders, "tex");
    uniform_color = glGetUniformLocation(shp_text_shaders, "uniform_color");

    /* Set up Vertex Buffer Object for text rendering. */
    glGenBuffers(1, &vbo_text);

    /* Generate texture atlas. */
    create_texture_atlas();

    /* Set global transformation uniform location. */
    uniform_tranformation = glGetUniformLocation(shp_basic_shaders,
                                                 "transformation");

    // ========================================
    // == Display loop
    // ========================================

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glfwSwapInterval(1);
    double time_start = 0;
    double time_stop = 0;
    bool DISPLAY_PERFORMANCE = true;
    size_t FRAMERATE_LIMIT = 250;
    double TIME_FRAME_LIMIT = 1000 / FRAMERATE_LIMIT; // ms/frame.
    while(!glfwWindowShouldClose(window)){

        if (DISPLAY_PERFORMANCE) {
            time_start = glfwGetTime();
        }

        glClear(GL_COLOR_BUFFER_BIT);

        glfwPollEvents();
        process_events();

        glUseProgram(shp_basic_shaders);
        glBindVertexArray(VAO);
        glUniformMatrix4fv(uniform_tranformation,
                           1,
                           GL_TRUE,
                           ptr_m4(m4_transformation));
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glUseProgram(0);
        glBindVertexArray(0);

        render_text("Hej Amanda, vi har text!", -1.0, -0.8, shp_text_shaders);

        if (DISPLAY_PERFORMANCE) {
            time_stop = glfwGetTime();
            double time_diff = time_stop-time_start;
            double time_diff_millis = time_diff * 1e3;
            if (time_diff_millis < TIME_FRAME_LIMIT) {
                struct timespec diff = {
                    .tv_sec = 0,
                    .tv_nsec = 500,
                };
                nanosleep(&diff, NULL);
            }
            display_number(1e3/time_diff_millis, 1.0, -0.8, shp_text_shaders);
        }

        glfwSwapBuffers(window);
    }
    glfwTerminate();
    free(mappings);
    Pa_Terminate();
    glfwTerminate();
    // Free allocated parameters.
    struct free_param_node * ptr = queue_params;
    while(ptr != NULL){
        struct free_param_node * temp = ptr;
        ptr = ptr->next;
        temp->free(temp->data);
        free(temp);
    }
}
