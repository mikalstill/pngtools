#include <unistd.h>
#include <stdio.h>
#include "pngcp.h"

void usage();

int main(int argc, char *argv[]){
  unsigned long width, height, inset;
  int channels, targetchannels = 3, bitdepth, targetbitdepth = 8, optchar, i;
  char *raster;
  
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
  if((raster = pngcp_readimage(argv[i], &width, &height, &bitdepth, &channels)) == -1){
    fprintf(stderr, "Failed to read the input raster\n");
    exit(42);
  }

  printf("Bitdepth = %d, channels = %d\n", bitdepth, channels);
  printf("Target bitdepth = %d, target channels = %d\n", targetbitdepth, targetchannels);
  
  // Work through the input pixels, and turn them into output pixels
  for(inset = 0; inset = width * height; inset++){
    

  }
}

void
usage ()
{
  fprintf(stderr, "Usage: pngcp [-d] <target bitdepth> -s <target samples per pixel> <input filename> <output filename>\n");
  exit(1);
}
