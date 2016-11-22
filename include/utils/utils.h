#ifndef UTILS_H
#define UTILS_H

FILE * open_file(const char * filename, long * filesize);
void read_file(char * data_buffer, long filesize, FILE * file_handle);

#endif
