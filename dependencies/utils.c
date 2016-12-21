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


size_t strip_comments(
                      char * temp_buffer,
                      size_t data_size,
                      size_t * return_rows
                     )
    /* Strip commands and return the new size without commands. */
{
        // Count rows when there are no comments.
        size_t rows = 1;

        // Remove comments in place.
        char * last_valid = NULL;
        char prev, current = ' ';
        int comment_flag = 0;
        size_t new_size = 0;
        for (size_t i=1; i<data_size; i++) {
            current = temp_buffer[i];
            prev = temp_buffer[i-1];
            // If comment, skip the chars until newline.
            if (comment_flag) {
                if (current == '\n') {
                    if (last_valid == NULL || *(last_valid-1) == '\n') { // Whole line comment.
                        i++; // Advance prev past the newline.
                    }
                    comment_flag = 0;
                    continue;
                }
                continue;
            }
            // Is this a comment?
            if( current == '/' && prev == '/') {
                comment_flag = 1;
                continue;
            }
            if (prev != '\n' && isspace(prev)) { // Strip all whitespace.
                continue;
            }
            // Copy the previous character to last_valid and increase the pointer.
            if (last_valid == NULL) {
                last_valid = temp_buffer;
            }
            *last_valid++ = prev;
            // Increment rows if there was a newline.
            if (prev == '\n') {
                rows++;
            }
            // Keep track of new size.
            new_size++;
            // Update previous character.
            prev = current;
        }

        // Add NULL at end.
        *last_valid = '\0';

        // Write to return_rows.
        *return_rows = rows;

        return new_size;
}

char * get_text_without_comments(const char * filename, Point_Data * info)
{
    // Load point data.
    size_t data_size = 0;
    FILE * point_file = open_file(filename, &data_size);

    const char * error_base = "[!] get_text_without_comments:";

    // Allocate temporary buffer on the heap.
    char * temp_buffer = malloc(sizeof(char)*(data_size+1));
    if (!temp_buffer) {
        fprintf(stderr, "%s could not allocate enough memory (%zu) for %s.\n",
                error_base,
                data_size,
                filename);
        exit(1);
    }

    // Read data into temp_buffer.
    read_file(temp_buffer, data_size, point_file);

    size_t new_size = 0;
    size_t rows = 0;

    new_size = strip_comments(temp_buffer, data_size, &rows);

    /* Count and return number of elements in file. */
    size_t elements = 1; // Doesn't count last comma, will be short otherwise.

    char prev = ' ';
    for(size_t i=1; i<new_size; i++) {
        prev = temp_buffer[i-1];
        if (prev == ',') { // Avoid counting training separator.
            elements++;
        }
    }

    info->elements = elements;

    // Add number of rows.
    info->rows = rows;

    return temp_buffer;
}


void utils_load_data(Point_Data * info, float * buffer, char * temp_buffer)
{
    // Define delimiter and current_token.
    const char * current_token = "";
    const char * delimiter = ",";
    current_token = strtok(temp_buffer, delimiter);

    // Iterate over and convert all the elements in the data.
    for (size_t i = 0; i<info->elements; i++) {
        float converted = atof(current_token);
        buffer[i] = converted;
        current_token = strtok(NULL, delimiter);
    }

    // Bind buffer to info->data.
    info->data = buffer;
}

Point_Data * load_data(const char * filename)
    /* External function for loading data from a text file to a float array.
     * Use old utils_load_data to load data into a heap buffer. */
{
    // Allocate point data on the heap.
    Point_Data * point_data = malloc(sizeof(Point_Data));
    // Strip the comments from the data file.
    char * temp_buffer = get_text_without_comments(filename, point_data);
    // Allocate memory for vertex data.
    float * vertex_buffer = malloc(sizeof(float) * point_data->elements);
    // Parse float in temp_buffer and store in point data.
    utils_load_data(point_data, vertex_buffer, temp_buffer);
    // Free temp buffer.
    free(temp_buffer);
    // Return pointer to point data.
    return point_data;
}

bool logic_main(float left_side, comparison_type comp, float right_side) {
    // Main logic function for comparing values.


    switch(comp)
    {
        case GT: return left_side > right_side; break;
        case GTEQ: return left_side >= right_side; break;
        case LT: return left_side < right_side; break;
        case LTEQ: return left_side <= right_side; break;
        case EQ: return left_side == right_side; break;
        default:
             fprintf(stderr, "[!] unknown case hit in login_main(), aborting");
             exit(1);
             break;
    }
}

char * generic_src(const char * prefix, const char * filename)
    /* Generic version of returning string concatenation from a prefix and a
     * filename. Used internally to represent more specialized exposed
     * functions. */
{
    size_t prefix_size = strlen(prefix);
    size_t filename_size = strlen(filename);

    size_t text_size = (prefix_size+filename_size+1)*sizeof(char);
    char * buffer = malloc(text_size);
    // Terminates string automatically.
    snprintf(buffer, text_size, "%s%s", prefix, filename);
    return buffer;
}

char * data_src(const char * filename)
    /* Return data source path. */
{
    return generic_src(DATA_SRC, filename);
}

char * shader_src(const char * filename)
    /* Return shader source path. */
{
    return generic_src(SHADER_SRC, filename);
}

char * texture_src(const char * filename)
    /* Return texture source path. */
{
    return generic_src(TEXTURE_SRC, filename);
}
