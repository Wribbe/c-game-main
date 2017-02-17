#include <stdlib.h>
#include <stdio.h>

void malloc_array(char ** pointer, size_t size) {
    *pointer = malloc(size+1);
    if (*pointer == NULL) {
        fprintf(stderr, "Could not allocate memory, aborting.\n");
        exit(0);
    }
}

void assign_chars(size_t row_size, size_t num_rows, char ** array) {

    char letters[] = "abcdefghijklmnopqrstuvwxyz";
    // Dereference the ** char pointer to get a pointer inside the char array.
    char * row_pointer = *array;
    char * letter_pointer;

    size_t current_row = 0;
    // Don't count the auto included '\0' char in the string.
    size_t num_letters = sizeof(letters)-1;

    /* char ** => pointer till en char[]
     * *(char **) => char * => char[]
     * *(char* / char[]) => char
     */

    while (current_row < num_rows) {
        letter_pointer = row_pointer;
        for (size_t i = 0; i < row_size; i++) {
            size_t letter_index = i%num_letters;
            char current_letter = letters[letter_index];
            // Replace included NULL char from string to \n.
            if (i == row_size-1) { // Make it a pretty box.
                current_letter = '\n';
            }
            *letter_pointer = current_letter;
            letter_pointer++;
        }
        row_pointer += row_size;
        current_row++;
    }

    /* How does the pointer move?
     *
     *  | -- 10 -- |                       Second loop.      End of loop.
     *   v (row_pointer == letter_pointer)  ----------
     *   ---------- <- row.                 v (row_p + 10)   ----------
     *   ----------                         ----------       ----------
     *   ----------                         ----------       ----------
     *   ----------                         ----------       ----------  v
     *   ----------'\0'                     ----------'\0'   ----------'\0'
     *
     */

    // Null terminate array.
    *(row_pointer) = '\0';
    // ### Tried ###:
    //  *(row_pointer + row_size) = '\0'; // 9 bytes after 51 blocks allocated.
    //  *array[51] = '\0'; // This works fine.
    // #############
}

int main(void) {

    /* Create a array through malloc and a char ** attribute in another
     * function. Iterate over that array and assign values to that array
     * through pointer arithmetic. Null terminate the array and the print it as
     * a string through printf.
     */

    char * char_array;
    size_t row_size = 70;
    size_t num_rows = 10;
    size_t size = row_size * num_rows * sizeof(char);

    malloc_array(&char_array, size);
    assign_chars(row_size, num_rows, &char_array);
    printf("%s\n", char_array);

    free(char_array);
}
