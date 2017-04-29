#include <stdlib.h>
#include <stdio.h>

#define buffer_char_size  1024
char buffer_char[buffer_char_size];

int mkdir(char * dirpath)
    /* Create a directory, skip if already exists. */
{
    snprintf(buffer_char, buffer_char_size, "mkdir -p %s", dirpath);
    return system(buffer_char);
}

int main(void)
{
    mkdir("test_dir");
    return 0;
}
