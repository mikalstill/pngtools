#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include "pngcp.h"

void usage();

int main(int argc, char *argv[]){
  unsigned long width, height, inset, outset;
  int channels, targetchannels = -1, bitdepth, targetbitdepth = -1, optchar, i,
    bytedepth, targetbytedepth;
  char *input, *output;
  float scalefactor;
  
  i = 1;
  while ((optchar = getopt (argc, argv, "d:s:")) != -1)
    {
      switch (optchar)
	{
	case 'd':
	  targetbitdepth = atoi(optarg);
	  i += 2;
	  break;
	  
	case 's':
	  targetchannels = atoi(optarg);
	  i += 2;
	  break;

	case '?':
	default:
	  usage ();
	  break;
	}
    }

  // Determine if we were given a filename on the command line
  if (argc < 2)
    usage ();

  // Colour depth is the number of bits per sample
  // Bit depth is the number of samples per pixel
  if((input = pngcp_readimage(argv[i], &width, &height, &bitdepth, &channels)) == -1){
    fprintf(stderr, "Failed to read the input raster\n");
    exit(42);
  }
  if(targetbitdepth == -1) targetbitdepth = bitdepth;
  if(targetchannels == -1) targetchannels = channels;

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
    exit(42);
  }

  printf("Bitdepth = %d, channels = %d, bytedepth = %d\n", bitdepth, channels, bytedepth);
  printf("Target bitdepth = %d, target channels = %d, target bytedepth = %d\n", 
	 targetbitdepth, targetchannels, targetbytedepth);
  
  // Determine how much each sample has to be scaled by to get to the new bitdepth
  scalefactor = (float) (pow(2.0, (double) targetbitdepth) - pow(2.0, (double) bitdepth)) / 
    pow(2.0, (double) bitdepth);
  printf("Scaling factor is %f - %f / %f = %f\n", (float) pow(2.0, (double) targetbitdepth),
	 (float) pow(2.0, (double) bitdepth), (float) pow(2.0, (double) bitdepth), scalefactor);

  // Work through the input pixels, and turn them into output pixels
  outset = 0;
  for(inset = 0; inset < width * height * channels; inset += bytedepth){
    // todo_mikal: This will only work for images with a bytedepth of one
    output[outset] = input[inset] * scalefactor;
    outset += targetbytedepth;
  }

  // Now push the raster into the output file
  if(pngcp_writeimage(argv[i + 1], width, height, targetbitdepth, targetchannels, output) < 0){
    fprintf(stderr, "Error writing the output file\n");
    exit(42);
  }
}

void
usage ()
{
  fprintf(stderr, "Usage: pngcp [-d] <target bitdepth> -s <target samples per pixel> <input filename> <output filename>\n");
  exit(1);
}
