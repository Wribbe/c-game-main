/* Utility file for c-main-game. */
#include <stdio.h>

FILE * open_file(const char * filename, long * filesize) {
    /* Try to open file:
     *  - If file exists, return file handle and write size of file to filesize
     *    attribute.
     *  - If the file does not exist, return NULL.
     *  */

    FILE * file_handle = fopen(filename, "rb");
    if (!file_handle) { // Return NULL handle.
        return file_handle;
    }

    // Seek end of file.
    fseek(file_handle, 0, SEEK_END);
    // Add one for newline.
    *filesize = ftell(file_handle)+1;
    // Rewind file pointer.
    rewind(file_handle);

    return file_handle;
}
