#include <ctype.h>
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
main                            (int argc, const char ** argv)
    /* Read text and spit it back again. */
{
    if (argc < MINARGS) {
        const char * fmt_use = "%s%s%s\n";
        const char * name = argv[0];
        fprintf(stderr, fmt_use, "Usage: ", name," filename.c [> output.c]");
        exit(EXIT_FAILURE);
    }

    const char * filename = argv[1];
    FILE * filep = fopen(filename, "r");

    if (filep == NULL) {
        DIE("No such file: \"%s\", aborting.\n", filename);
    }

    size_t size_buffer_beginning = 1024;
    char * buffer_beginning = malloc(size_buffer_beginning * sizeof (char));
    char * buffer_start = buffer_beginning;
    char * buffer_stop = buffer_beginning;
    char * buffer_end = buffer_beginning + size_buffer_beginning;

    bool IN_FUNCTION_COMMENT = false;
    bool IN_LINE_COMMENT = false;
    bool IN_MULLINE_COMMENT = false;

    int current = fgetc(filep);
    if (current == EOF) {
        DIE("Was nothing to read in file: \"%s\"\n", filename);
    }

    int next = fgetc(filep);
    for (; current != EOF; current=next, next=fgetc(filep)) {
        // Character manipulations.
        if (isspace(next) && isspace(current)) {
            // Compress whitespace.
            continue;
        } else if (current == ' ' && next == '(') {
            // No space between name and parenthesis.
            continue;
        } else if (current == '\n') {
            // Replace newlines with space.
            current = ' ';
        }

        *buffer_stop++ = current;
        if (buffer_stop+1 >= buffer_end) {
            printf("%.*s", buffer_stop-buffer_start, buffer_start);
            buffer_start = buffer_stop = buffer_beginning;
        }
//        if (current == '\n' ) {
//           if (buffer_stop < buffer_start) {
//               printf("%*c", buffer_end-buffer_stop, buffer_stop);
//               printf("%*c", buffer_start-buffer_beginning, buffer_beginning);
//           } else {
//               *buffer_stop = '\0';
//               printf("%s", buffer_start);
//           }
//           buffer_stop = buffer_start;
//           current = ' ';
//        }

//        set_parsing_status(c, prev, &IN_LINE_COMMENT, &IN_MULLINE_COMMENT);
//
//        if (IN_LINE_COMMENT || IN_MULLINE_COMMENT) {
//            *buffer_stop++ = c;
//            if (buffer_stop > buffer_beginning + size_buffer_beginning) {
//                buffer_stop = buffer_beginning;
//            }
//        } else if (pos_buffer > buffer_line) {
//            *buffer_stop = '\0';
//            printf("\n ---> Found comment: \n%s\n", buffer_line);
//            pos_buffer = buffer_line;
//        }
//        if (IN_LINE_COMMENT && c == '\n') {
//            IN_LINE_COMMENT = false;
//        }
    }

    if (buffer_stop > buffer_start) {
        printf("%.*s", buffer_stop-buffer_start, buffer_start);
        buffer_start = buffer_stop = buffer_beginning;
    }

    fclose(filep);
    free(buffer_beginning);
}
