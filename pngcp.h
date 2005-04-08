#ifndef PNGCP_H
#define PNGCP_H

char *readimage(char *filename, unsigned long *width, unsigned long *height, 
		      int *bitdepth, int *channels);

int writeimage(char *filename, unsigned long width, unsigned long height, 
		     int bitdepth, int channels, char *raster);

char *inflateraster(char *input, unsigned long width, unsigned long height, 
		    int bitdepth, int targetbitdepth, 
		    int channels, int targetchannels);
#endif
