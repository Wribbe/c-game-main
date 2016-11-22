#ifndef UTILS_H
#define UTILS_H

FILE * open_file(const char * filename, size_t * filesize);
size_t read_file(char * data_buffer, size_t filesize, FILE * file_handle);

#endif
