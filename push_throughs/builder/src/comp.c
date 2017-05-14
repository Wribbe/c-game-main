#include <dirent.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define SIZE(x) sizeof x / sizeof x[0]

const char * SOURCE_EXTENSION = ".c";
const char * OBJECT_EXTENSION = ".o";
const char * PATH_SEP = "/";

const char * FLAGS = "-lX11 -lGL -lglfw3 -lXrandr -lm -ldl -lpthread -lrt -lXinerama -lXcursor -lXxf86vm";
const char * FLAGS_DEF = "-Wall -std=c11 -pedantic -g -Iinclude";
const char * FLAGS_LIBS = "libglfw3.a";

enum dirpos {
    BIN,
    LIB,
    SRC,
    OBJ,
    TOTAL_DIRS,
};
const char * dirs[TOTAL_DIRS];

int endswith(const char * string, const char * ending)
{
    char * rightmost_dot = strrchr(string, '.');
    if(rightmost_dot && strcmp(rightmost_dot, ending) == 0) {
        return(rightmost_dot - string);
    }
    return 0;
}

void setup_folders(void)
{
    dirs[BIN] = "bin";
    dirs[LIB] = "lib";
    dirs[SRC] = "src";
    dirs[OBJ] = "obj";

    int mode = 0700;

    struct stat st = {0};
    for (size_t i = 0; i<TOTAL_DIRS; i++) {
        if(stat(dirs[i], &st) == -1) {
            mkdir(dirs[i], mode);
        }
    }
}

void run(const char * format, ...)
{
    size_t command_size = 1024;
    char buffer[command_size];

    va_list args;

    va_start(args, format);
    int chars_written = vsnprintf(buffer, command_size-1, format, args);
    va_end(args);

    buffer[chars_written] = '\0';
    printf("command == %s\n", buffer);

    system(buffer);
}


int main(void)
{
    setup_folders();

    char * objects[1024];
    char ** current_obj = &objects[0];
    size_t size_total_objects_string = 0;

    struct dirent * dir = NULL;
    DIR * d = opendir("lib");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            const char * dir_name = dir->d_name;
            int dot_pos = endswith(dir_name, SOURCE_EXTENSION);
            if (dot_pos) {

                size_t size_object_name = dot_pos+strlen(OBJECT_EXTENSION)+1;
                size_t size_total_object_path = strlen(dirs[OBJ]) + \
                                                strlen(PATH_SEP) + \
                                                size_object_name + 1;

                // Add current path size to total size.
                size_total_objects_string += size_total_object_path+1;

                // Allocate memory for current object path.
                *current_obj = malloc(size_total_object_path*sizeof(char));

                // Write path to current obj in object list.
                snprintf(*current_obj, size_total_objects_string, "%s%s%.*s%s",
                         dirs[OBJ],
                         PATH_SEP,
                         dot_pos,
                         dir_name,
                         OBJECT_EXTENSION
                    );

                // Advance current object pointer.
                current_obj++;

                // Run object compilation command.
                run("gcc -c lib/%s -o %s%s%.*s%s -g %s %s",
                        dir_name,
                        dirs[OBJ],
                        PATH_SEP,
                        dot_pos,
                        dir_name,
                        OBJECT_EXTENSION,
                        FLAGS_DEF,
                        FLAGS
                   );
                }
            }
        }
        closedir(d);
    d = opendir("src");
    if (d) {
        char objects_line[size_total_objects_string];
        char * pointer_object_line = objects_line;
        for (char ** pointer = &objects[0]; pointer < current_obj; pointer++) {
            pointer_object_line += sprintf(pointer_object_line, "%s ", *pointer);
        }
        *pointer_object_line = '\0';
        while ((dir = readdir(d)) != NULL) {
            const char * dir_name = dir->d_name;
            int dot_pos = endswith(dir_name, SOURCE_EXTENSION);
            if (dot_pos) {
                if (strcmp(dir_name, "comp.c") == 0) {
                    run("gcc src/%s -o %s%s%.*s %s %s",
                            dir_name,
                            dirs[BIN],
                            PATH_SEP,
                            dot_pos,
                            dir_name,
                            FLAGS_DEF,
                            FLAGS_LIBS
                       );
                } else {
                    run("gcc src/%s -o %s%s%.*s %s -g %s %s",
                            dir_name,
                            dirs[BIN],
                            PATH_SEP,
                            dot_pos,
                            dir_name,
                            objects_line,
                            FLAGS_DEF,
                            FLAGS,
                            FLAGS_LIBS
                       );
                }
            }
        }
        closedir(d);
    }
    for (char ** pointer = &objects[0]; pointer < current_obj; pointer++) {
        free(*pointer);
    }
    return(EXIT_SUCCESS);
}
