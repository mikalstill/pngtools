#ifndef PNGCP_H
#define PNGCP_H

#include <png.h>

png_byte *readimage(char *filename, unsigned long *width, unsigned long *height,
		      int *bitdepth, int *channels);

int writeimage(char *filename, unsigned long width, unsigned long height,
		     int bitdepth, int channels, png_byte *raster);

png_byte *inflateraster(png_byte *input, unsigned long width,
		    unsigned long height,
		    int bitdepth, int targetbitdepth,
		    int channels, int targetchannels);
#endif
