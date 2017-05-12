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
const char * PATH_SEP = "/";

const char * FLAGS = "libglfw3.a -lX11 -lGL -lglfw3 -lXrandr -lm -ldl -lpthread -lrt -lXinerama -lXcursor -lXxf86vm";

enum dirpos {
    BIN,
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
    struct dirent * dir = NULL;
    DIR * d = opendir(".");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            const char * dir_name = dir->d_name;
            int dot_pos = endswith(dir_name, SOURCE_EXTENSION);
            if (dot_pos) {
                if (strcmp(dir_name, "comp.c") == 0) {
                    run("gcc %s -o %s%s%.*s -g",
                            dir_name, dirs[BIN],
                            PATH_SEP,
                            dot_pos,
                            dir_name
                       );
                } else {
                    run("gcc %s -o %s%s%.*s -g %s",
                            dir_name, dirs[BIN],
                            PATH_SEP,
                            dot_pos,
                            dir_name,
                            FLAGS
                       );
                }
            }
        }
        closedir(d);
    }
    return(EXIT_SUCCESS);
}
