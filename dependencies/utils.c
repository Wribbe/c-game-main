/* Utility file for c-main-game. */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utils/utils.h"

FILE * open_file(const char * filename, size_t * filesize)
{
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
    *filesize = ftell(file_handle);
    // Rewind file pointer.
    rewind(file_handle);

    return file_handle;
}

size_t read_file(char * data_buffer, size_t filesize, FILE * file_handle)
{
    /* Read the file to data_buffer and append newline. */

    // Read data.
    size_t read = fread(data_buffer, filesize, 1, file_handle);
    // Add newline to end.
    data_buffer[filesize] = '\0';
    fclose(file_handle);
    return read;
}

void load_data(Point_Data * info, float * buffer, const char * filename)
{
    /* Function servers two purposes.
     *  - buffer == NULL:
     *      Count and populate the info struct with the number of elements and
     *      rows that are present in the data.
     *  - buffer != NULL:
     *      Outbound buffer has been provided, store data in the buffer and
     *      return, no further processing of the data done.
     */

    // Load point data.
    size_t data_size = 0;
    FILE * point_file = open_file(filename, &data_size);

    // Always allocate temporary buffer on stack.
    char temp_buffer[data_size];

    // Read data into temp_buffer.
    read_file(temp_buffer, data_size, point_file);

    // Process as floats if there was a buffer supplied.
    if (buffer != NULL) {

        // Define delimiter and current_token.
        const char * current_token = "";
        const char * delimiter = ",";
        current_token = strtok(temp_buffer, delimiter);

        // Iterate over and convert all the elements in the data.
        for (int i = 0; i<info->elements; i++) {
            float converted = atof(current_token);
            buffer[i] = converted;
            current_token = strtok(NULL, delimiter);
        }

        return; // Done second pass through.
    }

    int elements = 1; // Doesn't count last comma, will be short otherwise.
    int rows = 0;

    int element_flag = 0;

    char current = 'a';
    for(size_t i=0; i<data_size; i++) {
        current = temp_buffer[i];
        if (current == '\n') {
            rows++;
            continue;
        }
        // Next element after flag was not a newline or end of buffer.
        if (element_flag) {
            element_flag = 0;
            elements++;
        }
        if (current == ',') {
            element_flag = 1;
        }
    }

    // First time through with buffer == NULL, populate info.
    info->elements = elements;
    info->rows = rows;
}
