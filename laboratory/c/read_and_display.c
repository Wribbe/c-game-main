#include <stdlib.h>
#include <stdio.h>

#include "utils/utils.h"

int main(int argc, char ** argv) {

    const char * usage_error = "[!] Usage: ./read_and_display <filename>.\n";
    const char * fmt_file_error = "[!]: Could not find file: %s.\n";

    if (argc < 2) {
        fprintf(stderr, usage_error);
        return EXIT_FAILURE;
    }

    const char * filename = argv[1];

    long filesize = 0;

    // Open file.
    FILE * file_handle = open_file(filename, &filesize);

    if (!file_handle) {
        fprintf(stderr, fmt_file_error, filename);
        return EXIT_FAILURE;
    }

    // Allocate memory for data.
    char data_buffer[filesize+1];

    // Read data.
    read_file(data_buffer, filesize, file_handle);

    // Print read data.
    printf("Read data from %s:\n", filename);
    printf("%s", data_buffer);
}
