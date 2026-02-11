#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>
#include <unistd.h>

png_byte *readimage(char *filename, png_uint_32 *width, png_uint_32 *height,
		int *bitdepth, int *channels);

png_byte *readimage(char *filename, png_uint_32 *width, png_uint_32 *height,
		      int *bitdepth, int *channels){
  FILE *image;
  png_uint_32 i, rowbytes;
  png_structp png;
  png_infop info;
  png_bytepp row_pointers = NULL;
  unsigned char sig[8];
  png_byte *raster;
  int colourtype;

  // Open the file
  if ((image = fopen (filename, "rb")) == NULL){
    fprintf(stderr, "Could not open the specified PNG file.");
    goto error;
  }

  // Check that it really is a PNG file
  fread(sig, 1, 8, image);
  if(!png_sig_cmp(sig, 0, 8) == 0){
    fprintf(stderr, "This file is not a valid PNG file\n");
    goto error;
  }

  // Start decompressing
  if((png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, 
				   NULL, NULL)) == NULL){
    fprintf(stderr, "Could not create a PNG read structure (out of memory?)");
    goto error;
  }

  if((info = png_create_info_struct(png)) == NULL){
    fprintf(stderr, "Could not create PNG info structure (out of memory?)");
    png_destroy_read_struct (&png, &info, NULL);
    goto error;
  }

  // If pnginfo_error did not exit, we would have to call 
  // png_destroy_read_struct
  if(setjmp(png_jmpbuf(png))){
    fprintf(stderr, "Could not set PNG jump value");
    goto error;
  }

  // Get ready for IO and tell the API we have already read the image signature
  png_init_io(png, image);
  png_set_sig_bytes(png, 8);
  png_read_info(png, info);
  png_get_IHDR(png, info, width, height, bitdepth, &colourtype, NULL, 
	       NULL, NULL);

  if(*bitdepth < 8)
    png_set_packing(png);

  if (colourtype == PNG_COLOR_TYPE_PALETTE)
    png_set_expand (png);

  // The channels bit has to be after here, so that the number of channels within the
  // palette is correctly reported...
  //png_set_strip_alpha (png);
  png_read_update_info (png, info);
  *channels = png_get_channels(png, info);
  
  rowbytes = png_get_rowbytes (png, info);
  if((row_pointers = malloc (*height * sizeof (png_bytep))) == NULL){
    fprintf(stderr, "Could not allocate memory\n");
    goto error;
  }

  // Space for the bitmap
  if((raster = malloc ((rowbytes * *height) + 1)) == NULL){
    fprintf(stderr, "Could not allocate memory\n");
    goto error;
  }

  // Get the image bitmap
  for (i = 0; i < *height; ++i)
    row_pointers[i] = raster + (i * rowbytes);
  png_read_image (png, row_pointers);
  goto cleanup;

 error:
  free(raster);
  raster = NULL;

 cleanup:
  if(row_pointers != NULL) free(row_pointers);
  png_read_end (png, NULL);
  fclose (image);
  png_destroy_read_struct (&png, &info, NULL);

  // And return the raster
  return raster;
}
