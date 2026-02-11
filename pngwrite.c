#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>
#include <unistd.h>
#include "pngcp.h"

int
writeimage(const char *filename, png_uint_32 width, png_uint_32 height, int bitdepth, int channels,
           png_byte *raster)
{
  FILE *volatile image = NULL;
  png_structp png = NULL;
  png_infop info = NULL;
  png_bytepp volatile row_pointers = NULL;
  png_uint_32 i;
  int rowbytes;
  volatile int ret = -1;

  if ((image = fopen(filename, "wb")) == NULL)
    {
      fprintf(stderr, "Could not open the output image\n");
      goto cleanup;
    }

  // Determine how many bytes each row will consume
  rowbytes = bitdepth / 8;
  if (bitdepth % 8 != 0)
    rowbytes++;
  rowbytes *= channels;
  rowbytes *= width;

  // Convert the raster into a series of row pointers
  if ((row_pointers = malloc(height * sizeof(png_bytep))) == NULL)
    {
      fprintf(stderr, "Could not allocate memory\n");
      goto cleanup;
    }

  for (i = 0; i < height; ++i)
    row_pointers[i] = raster + (i * rowbytes);

  // Get ready for writing
  if ((png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) == NULL)
    {
      fprintf(stderr, "Could not create write structure for PNG (out of memory?)\n");
      goto cleanup;
    }

  // The pixels are expanded to the nearest byte
  png_set_expand(png);

  // Get ready to specify important stuff about the image
  if ((info = png_create_info_struct(png)) == NULL)
    {
      fprintf(stderr, "Could not create PNG info structure for writing (out of memory?)\n");
      goto cleanup;
    }

  if (setjmp(png_jmpbuf(png)))
    {
      fprintf(stderr, "Could not set the PNG jump value for writing\n");
      goto cleanup;
    }

  // This is needed before IO will work (unless you define callbacks)
  png_init_io(png, image);

  // Derive the PNG color type from the number of channels
  int colourtype;
  switch (channels)
    {
    case 1:
      colourtype = PNG_COLOR_TYPE_GRAY;
      break;
    case 2:
      colourtype = PNG_COLOR_TYPE_GRAY_ALPHA;
      break;
    case 3:
      colourtype = PNG_COLOR_TYPE_RGB;
      break;
    case 4:
      colourtype = PNG_COLOR_TYPE_RGB_ALPHA;
      break;
    default:
      fprintf(stderr, "Unsupported channel count: %d\n", channels);
      goto cleanup;
    }

  // Define important stuff about the image
  png_set_IHDR(png, info, width, height, bitdepth, colourtype, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  png_write_info(png, info);

  // Write the image out
  png_write_image(png, row_pointers);

  // Success
  png_write_end(png, info);
  ret = 0;

cleanup:
  if (png != NULL)
    png_destroy_write_struct(&png, &info);
  free(row_pointers);
  if (image != NULL)
    fclose(image);
  return ret;
}
