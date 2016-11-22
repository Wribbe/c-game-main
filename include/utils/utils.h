#ifndef UTILS_H
#define UTILS_H

#define data_src(x) "data/"x

FILE * open_file(const char * filename, size_t * filesize);
size_t read_file(char * data_buffer, size_t filesize, FILE * file_handle);

typedef struct {
    int rows;
    int elements;
    void * data;
} Point_Data;

void load_data(Point_Data * info, float * buffer, const char *  filename);

#endif
