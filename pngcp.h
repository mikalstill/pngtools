#ifndef PNGCP_H
#define PNGCP_H

#include <png.h>

png_byte *readimage(const char *filename, png_uint_32 *width, png_uint_32 *height, int *bitdepth,
                    int *channels);

int writeimage(const char *filename, png_uint_32 width, png_uint_32 height, int bitdepth,
               int channels, png_byte *raster);

png_byte *inflateraster(png_byte *input, png_uint_32 width, png_uint_32 height, int bitdepth,
                        int targetbitdepth, int channels, int targetchannels);
#endif
