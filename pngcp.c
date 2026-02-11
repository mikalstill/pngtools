#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include "pngcp.h"

/******************************************************************************
DOCBOOK START

FUNCTION pngcp
PURPOSE create a new PNG file, having changed some attributes

SYNOPSIS START
pnginfo [-d <depth>] [-s <samples per pixel>] <input filename> <output filename>
SYNOPSIS END

DESCRIPTION START
The <command>pngcp</command> creates a new PNG file using the image data from the input file. The output file will have the bitdepth and number of samples per pixel as specified on the command line. Both changes can be applied simultaneously. There are limits on what is a valid combination imposed by the PNG specification -- <command>pngcp</command> will inform you of invalid combinations.
DESCRIPTION END

RETURNS Nothing

EXAMPLE START
%bash: pngcp toucan.png new.png
EXAMPLE END
SEEALSO libpng libtiff tiffcp pngchunkdesc pnginfo
DOCBOOK END
******************************************************************************/

void usage();

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

void
usage()
{
  fprintf(stderr, "Usage: pngcp [-d <target bitdepth>] [-s <target samples per pixel>] <input "
                  "filename> <output filename>\n");
  exit(1);
}
