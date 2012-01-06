#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// Inflate a raster to a given pixel sample size
char *inflateraster(char *input, unsigned long width, unsigned long height, 
		    int bitdepth, int targetbitdepth, 
		    int channels, int targetchannels){
  float scalefactor;
  char *output;
  int bytedepth, targetbytedepth;
  unsigned int inset, outset;

  // Why are we here?
  if((channels == targetchannels) && (bitdepth == targetbitdepth))
    return input;

  // Calculate the byte depth
  bytedepth = bitdepth / 8;
  if(bitdepth % 8 != 0)
    bytedepth++;
  
  // Calculate the target byte depth
  targetbytedepth = targetbitdepth / 8;
  if(targetbitdepth % 8 != 0)
    targetbytedepth++;

  // Build the output raster
  if((output = (char *) malloc(width * height * targetchannels * targetbytedepth)) == NULL){
    fprintf(stderr, "Failed to allocate enough memory for output raster\n");
    return (char *) -1;
  }

  // Are we changing the bitdepth?
  if(bitdepth != targetbitdepth){
    // Determine how much each sample has to be scaled by to get to the new bitdepth
    scalefactor = (float) pow(2.0, (double) targetbitdepth)  / 
      (float) pow(2.0, (double) bitdepth);
    printf("Scaling factor is %f - %f / %f = %f\n", 
	   (float) pow(2.0, (double) targetbitdepth),
	   (float) pow(2.0, (double) bitdepth), 
	   (float) pow(2.0, (double) bitdepth), 
	   scalefactor);
    
    // Work through the input pixels, and turn them into output pixels
    outset = 0;
    for(inset = 0; inset < width * height * channels; inset += bytedepth){
      // todo_mikal: This will only work for images with a bytedepth of one
      output[outset] = input[inset] * scalefactor;
      outset += targetbytedepth;
    }
  }
  
  // Are we the number of channels?
  // todo_mikal: we can't do both of these changes in one pass at the moment...
  if(channels != targetchannels){
    printf("Expanding from %d channels to %d channels\n", channels, targetchannels);
    printf("Target byte depth is %d bytes\n", targetbytedepth);

    // Work through the input pixels, and turn them into output pixels
    outset = 0;
    for(inset = 0; inset < width * height * channels; inset += bytedepth){
      int i;

      // this wont work for just adding an alpha channel
      for(i = 0; i < targetchannels - channels + 1; i++){
	output[outset] = input[inset];
	outset += targetbytedepth;
      }
    }
  }

  return output;
}
