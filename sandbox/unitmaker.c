#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define MINARGS 2

#define ERROR(...) fprintf (stderr, "[!] Error: "__VA_ARGS__)
#define WARNING(...) fprintf (stderr, "[?] Warning: "__VA_ARGS__)
#define DIE(...) ERROR(__VA_ARGS__); exit(EXIT_FAILURE)

void
set_parsing_status(int c, int prev, bool * line, bool * mulline)
    /* Set parsing status for finding comments in code.
     * @Doc-start.
     * @end.
     */
{
    switch (c) {
        case '/':
            if (prev == '/') {
                *line = true;
            } else if (*mulline && prev == '*') {
                *mulline = false;
            }
            break;
        case '*':
            if (!*mulline && !*line && prev == '/') {
                *mulline = true;
            }
            break;
    }
}

void
main(int argc, const char ** argv)
    /* Read text and spit it back again. */
{
    if (argc < MINARGS) {
        fprintf(stderr, "%s%s%s\n", "Usage: ", argv[0], " filename.c [> output.c]");
        exit(EXIT_FAILURE);
    }

    const char * filename = argv[1];
    FILE * filep = fopen(filename, "r");

    if (filep == NULL) {
        DIE("No such file: \"%s\", aborting.\n", filename);
    }

    size_t size_buffer_line = 512;
    char buffer_line[size_buffer_line];
    char * pos_buffer = buffer_line;

    int c;
    int prev = -1;

    bool IN_FUNCTION_COMMENT = false;
    bool IN_LINE_COMMENT = false;
    bool IN_MULLINE_COMMENT = false;

    while ((c = fgetc(filep)) != EOF) {
        set_parsing_status(c, prev, &IN_LINE_COMMENT, &IN_MULLINE_COMMENT);
        if (IN_LINE_COMMENT || IN_MULLINE_COMMENT) {
            *pos_buffer++ = c;
        } else if (pos_buffer > buffer_line) {
            *pos_buffer = '\0';
            printf("\n ---> Found comment: \n%s\n", buffer_line);
            pos_buffer = buffer_line;
        }
        if (IN_LINE_COMMENT && c == '\n') {
            IN_LINE_COMMENT = false;
        }
        prev = c;
        printf("%c", c);
    }
}
