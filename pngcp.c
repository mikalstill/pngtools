#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include "pngcp.h"

static void usage(void);

int
main(int argc, char *argv[])
{
  png_uint_32 width, height;
  int channels, targetchannels = -1, bitdepth, targetbitdepth = -1, optchar, i;
  png_byte *input, *output;

  i = 1;
  while ((optchar = getopt(argc, argv, "d:s:")) != -1)
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
          usage();
          break;
        }
    }

  // Determine if we were given a filename on the command line
  if (argc < 2)
    usage();

  // Colour depth is the number of bits per sample
  // Bit depth is the number of samples per pixel
  if ((input = readimage(argv[i], &width, &height, &bitdepth, &channels)) == NULL)
    {
      fprintf(stderr, "Failed to read the input raster\n");
      exit(42);
    }
  if (targetbitdepth == -1)
    targetbitdepth = bitdepth;
  if (targetchannels == -1)
    targetchannels = channels;

  if ((output
       = inflateraster(input, width, height, bitdepth, targetbitdepth, channels, targetchannels))
      == NULL)
    {
      fprintf(stderr, "Failed to inflate the raster to the requested size\n");
      exit(42);
    }

  // Now push the raster into the output file
  if (writeimage(argv[i + 1], width, height, targetbitdepth, targetchannels, output) < 0)
    {
      fprintf(stderr, "Error writing the output file\n");
      exit(42);
    }
}

static void
usage(void)
{
  fprintf(stderr, "Usage: pngcp [-d <target bitdepth>] [-s <target samples per pixel>] <input "
                  "filename> <output filename>\n");
  exit(1);
}
