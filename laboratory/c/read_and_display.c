#include <stdlib.h>
#include <stdio.h>

int main(int argc, char ** argv) {

    const char * usage_error = "[!] Usage: ./read_and_display <filename>.\n";
    const char * fmt_file_error = "[!]: Could not find file: %s.\n";

    if (argc < 2) {
        fprintf(stderr, usage_error);
        return EXIT_FAILURE;
    }

    const char * filename = argv[1];

    // Open file.
    FILE * file_handle = fopen(filename, "rb");

    if (!file_handle) {
        fprintf(stderr, fmt_file_error, filename);
        return EXIT_FAILURE;
    }

    // Check size of data.
    fseek(file_handle, 0, SEEK_END);
    long filesize = ftell(file_handle);
    // Rewind file pointer.
    rewind(file_handle);

    // Allocate memory for data.
    char data_buffer[filesize+1];

    // Read data.
    fread(data_buffer, filesize, 1, file_handle);

    // End with a newline.
    data_buffer[filesize] = '\0';

    // Print read data.
    printf("Read data from %s:\n", filename);
    printf("%s", data_buffer);
}
