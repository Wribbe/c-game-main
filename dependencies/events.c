#include "GLFW/glfw3.h"
#include "events/events.h"

void callback_key(
                  GLFWwindow * window,
                  int key,
                  int scancode,
                  int action,
                  int mods
                 )
{
    printf("HELLO\n");
}
