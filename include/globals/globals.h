#ifndef GLOBALS_H
#define GLOBALS_H

#include <GLFW/glfw3.h>

#define NUM_KEYS 512

extern struct timespec start_time;
extern struct timespec end_time;

enum coord {
    X,
    Y,
    Z,
    NUM_COORD,
};

typedef enum global_index {
    GRAVITY,
    SPEED,
    glfw_time,
    TIMESTEP,
    JUMP_VELOCITY,
    PRINT_FPS,
    // Keep this one as last element.
    num_globals,
} global_index;

enum uniform_type {
    /* Strip the gl part of the glUniform* functions used. */
    UniformMatrix4fv,
    Uniform1f,
    NUM_UNIFORM_TYPES,
};

enum flag_type {
    AIRBORN,
    IS_JUMPING,
    GRAVITY_ON,
    KEEP_INSIDE_COLLISION,
};

enum component_list_type {
    CONTROLLABLE,
    NON_CONTROLLABLE,
    SCENE_COMPONENTS,
    NUM_COMPONENT_TYPES,
};

// Global variables.
extern float global_variables[];

struct uniform_data {
    const char * name;
    void * (*data_function)(struct component *);
    enum uniform_type type;
};

#define controlled_size 2
#define standard_size 1

// Global uniform sets.
extern struct uniform_data controlled_uniforms[];
extern struct uniform_data standard_uniforms[];

void setup_globals(void);
void global_init(void);

// Global pointers to different components.
struct component * components[NUM_COMPONENT_TYPES];
struct component * last_component[NUM_COMPONENT_TYPES];
struct component * controlled_component;

extern float timestep(void);
extern float gravity(void);
extern float speed(void);
extern float jump_velocity(void);
void set_timestep(float new_value);

#endif
