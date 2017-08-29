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
print_between (const char * a, const char * b)
    /* Print the string between the two pointers. */
{
    printf("%.s", b-a, a);
}

void
main (int argc, const char ** argv)
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

    char * last_whitsepace = buffer_beginning;
    char * backtrack_from_space = buffer_beginning;

    bool IN_FUNCTION_COMMENT = false;
    bool IN_LINE_COMMENT = false;
    bool IN_MULLINE_COMMENT = false;

    int current = fgetc(filep);
    if (current == EOF) {
        DIE("Was nothing to read in file: \"%s\"\n", filename);
    }
    int prev = current; // Doesn't really matter.
    int next = fgetc(filep);
    for (; current != EOF; prev=current, current=next, next=fgetc(filep)) {
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

        // Keep track of last whiespace.
        if (current == ' ') {
            last_whitsepace = buffer_stop;
        }

        // Hit a '(' where previous was not a space.
        if (current == '(' && !isspace(prev)) {
            // Iterate backwards until return value is found.
            backtrack_from_space = last_whitsepace;
            while (!isspace(*backtrack_from_space)) {
                if (backtrack_from_space-1 < buffer_start) {
                    // Wrapping around.
                    backtrack_from_space = buffer_end;
                }
                backtrack_from_space--;
            }
            print_between(backtrack_from_space, buffer_stop);
        }

        // Write current char to buffer and advance.
        *buffer_stop++ = current;
        if (buffer_stop+1 >= buffer_end) {
            print_between(buffer_start, buffer_stop);
            // Realign buffer pointers.
            buffer_start = buffer_stop = buffer_beginning;
        }
    }

    // Flush anything left in buffer.
    if (buffer_stop > buffer_start) {
        print_between(buffer_start, buffer_stop);
        // Realign buffer pointers.
        buffer_start = buffer_stop = buffer_beginning;
    }

    fclose(filep);
    free(buffer_beginning);
}
