#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>
#include <unistd.h>
#include "pngcp.h"

int pngcp_writeimage(char *filename, unsigned long width, unsigned long height, 
		      int bitdepth, int channels, char *raster){
  FILE *image;
  png_structp png;
  png_infop info;
  png_bytepp row_pointers = NULL;
  int i, rowbytes;

  if((image = fopen(filename, "wb")) == NULL){
    fprintf(stderr, "Could not open the output image\n");
    return -1;
  }

  // Determine how many bytes each row will consume
  rowbytes = bitdepth / 8;
  if(bitdepth % 8 != 0)
    rowbytes++;
  rowbytes *= channels;
  rowbytes *= width;

  // Convert the raster into a series of row pointers
  if((row_pointers = malloc (height * sizeof (png_bytep))) == NULL){
    fprintf(stderr, "Could not allocate memory\n");
    return -1;
  }

  for (i = 0; i < height; ++i)
    row_pointers[i] = raster + (i * rowbytes);

  // Get ready for writing
  if ((png =
       png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL,
                                NULL)) == NULL){
    fprintf(stderr, "Could not create write structure for PNG (out of memory?)\n");
    return -1;
  }

  // The pixels are expanded to the nearest byte
  png_set_expand (png);

  // Get ready to specify important stuff about the image
  if ((info = png_create_info_struct (png)) == NULL){
    fprintf(stderr,
            "Could not create PNG info structure for writing (out of memory?)\n");
    return -1;
  }

  if (setjmp (png_jmpbuf (png))){
    fprintf(stderr, "Could not set the PNG jump value for writing\n");
    return -1;
  }

  // This is needed before IO will work (unless you define callbacks)
  png_init_io(png, image);

  // We need to derive a PNG color type from the number of channels and bitdepth
  

  // Define important stuff about the image
  png_set_IHDR (png, info, width, height, bitdepth, PNG_COLOR_TYPE_RGB,
                PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
                PNG_FILTER_TYPE_DEFAULT);
  png_write_info (png, info);

  // Write the image out
  png_write_image (png, row_pointers);

  // Cleanup
  png_write_end (png, info);
  png_destroy_write_struct (&png, &info);
  fclose(image);
  return 0;
}
