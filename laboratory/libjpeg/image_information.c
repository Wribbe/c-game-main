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


int main(void) {
    const char * filename = texture_src("Dietrich.jpg");

    struct jpeg_decompress_struct cinfo = {0};

    struct my_error_mgr jerr;

    FILE * infile = NULL;
    int row_stride = 0;

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

    printf("height of picture: %d\n", cinfo.output_height);

    printf("Got to end!\n");
}
