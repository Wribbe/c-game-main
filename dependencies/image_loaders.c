#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>

#include "jpeglib/jpeglib.h"
#include "jpeglib/jerror.h"
#include "utils/utils.h"

struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

void my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}


int load_test(unsigned char ** jpeg_data, size_t * height, size_t * width) {
//unsigned char * load_test(size_t * height, size_t * width) {

    const char * filename = texture_src("Dietrich.jpg");
    //const char * filename = texture_src("green.jpg");

    struct jpeg_decompress_struct cinfo = {0};

    struct my_error_mgr jerr;

    FILE * infile = NULL;

    if ((infile = fopen(filename, "rb")) == NULL) {
        fprintf(stderr, "[!]: Can't open %s\n", filename);
        return 0;
    }

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;

    if (setjmp(jerr.setjmp_buffer)) {
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        return 0;
    }

    jpeg_create_decompress(&cinfo);

    jpeg_stdio_src(&cinfo, infile);

    jpeg_read_header(&cinfo, TRUE);

    jpeg_start_decompress(&cinfo);

    *width = cinfo.output_width;
    *height = cinfo.output_height;

    size_t row_stride = *width * cinfo.output_components;
    size_t size = *height * row_stride;

    printf("height: %lu width: %lu stride: %zu, size: %zu.\n", *height, *width, row_stride, size);

//    unsigned char * jpeg_data = malloc((size+1)*sizeof(unsigned char));
    *jpeg_data = malloc((size+1)*sizeof(unsigned char));
    if (jpeg_data == NULL) {
        fprintf(stderr, "Could not allocate memory, aborting.\n");
        return 0;
    }

    printf("components: %d\n", cinfo.output_components);

    unsigned char * temp_buffer = malloc(row_stride+1);

//    for (size_t i=0; i<size; i++) {
//        *(jpeg_data+i) = '-';
//    }

    // Read all scanlines.
    int lines_read = 0;
    //unsigned char * data_pointer;
    unsigned char * data_pointer = (*jpeg_data) + size;
    //data_pointer -= size - row_stride;
    while(cinfo.output_scanline < *height) {
        //data_pointer = (*jpeg_data + row_stride * cinfo.output_scanline);
        data_pointer -= row_stride;
        printf("output_scanline: %d, height: %zu\n", cinfo.output_scanline, *height);
        lines_read = jpeg_read_scanlines(&cinfo, &temp_buffer, 1);
//        printf("Written to temp_buffer; %s\n", temp_buffer);
        for (size_t i = 0; i<row_stride; i++) {
            data_pointer[i] = temp_buffer[i];
//            printf(" %c<-->%c ", temp_buffer[i], data_pointer[i]);
        }
//        temp_buffer[row_stride] = '\0';
//        *data_pointer = 'a';

//        unsigned char * temp_pointer = jpeg_data;
//        printf("current buffer: ");
//        while(temp_pointer != data_pointer) {
//            printf("%c", *temp_pointer);
//            temp_pointer++;
//        }
//        printf("lines read: %d data_pointer: %p current buffer: %s\n", lines_read, data_pointer, jpeg_data);
//        printf("temp_buffer: %s\n", temp_buffer);
//        data_pointer += lines_read * row_stride;
    }

    jpeg_finish_decompress(&cinfo);

//    jpeg_destroy_decompress(&cinfo);

    fclose(infile);

    *data_pointer = '\0';

    printf("Got to end!\n");

    return 1;
}
