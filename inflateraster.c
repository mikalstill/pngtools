#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <png.h>

// Read a sample value from a byte buffer (big-endian for 16-bit PNG samples)
static unsigned int read_sample(png_byte *data, int bytedepth){
  if(bytedepth == 2)
    return (data[0] << 8) | data[1];
  return data[0];
}

// Write a sample value to a byte buffer (big-endian for 16-bit PNG samples)
static void write_sample(png_byte *data, int bytedepth, unsigned int value){
  if(bytedepth == 2){
    data[0] = (value >> 8) & 0xFF;
    data[1] = value & 0xFF;
  } else {
    data[0] = value & 0xFF;
  }
}

// Map an output channel index to the corresponding input channel index.
// Returns -1 if the output channel has no input source and should be
// filled with the maximum sample value (e.g. opaque alpha).
//
// PNG channel layouts: 1=Gray, 2=Gray+Alpha, 3=RGB, 4=RGBA
static int map_channel(int out_ch, int channels, int targetchannels){
  int out_is_alpha = (targetchannels == 2 || targetchannels == 4) &&
                     (out_ch == targetchannels - 1);

  if(out_is_alpha){
    // Map to input alpha channel if available
    if(channels == 2) return 1;
    if(channels == 4) return 3;
    return -1;
  }

  // Gray -> RGB expansion: replicate gray to R, G, B
  if(channels <= 2 && targetchannels >= 3 && out_ch < 3)
    return 0;

  // Direct mapping when input has the channel
  if(out_ch < channels)
    return out_ch;

  return -1;
}

// Inflate a raster to a given pixel sample size
png_byte *inflateraster(png_byte *input, png_uint_32 width,
		    png_uint_32 height,
		    int bitdepth, int targetbitdepth,
		    int channels, int targetchannels){
  png_byte *output;
  int bytedepth, targetbytedepth;
  png_uint_32 pixel, npixels;
  float scalefactor;
  unsigned int maxval;

  // Nothing to do
  if((channels == targetchannels) && (bitdepth == targetbitdepth))
    return input;

  // Calculate byte depths
  bytedepth = (bitdepth + 7) / 8;
  targetbytedepth = (targetbitdepth + 7) / 8;

  // Build the output raster
  if((output = malloc(width * height * targetchannels * targetbytedepth)) == NULL){
    fprintf(stderr, "Failed to allocate enough memory for output raster\n");
    return NULL;
  }

  // Compute scaling factor for bitdepth changes, mapping the full
  // input range [0, 2^src-1] to the full output range [0, 2^dst-1]
  if(bitdepth != targetbitdepth){
    scalefactor = ((float)pow(2.0, (double)targetbitdepth) - 1.0) /
                  ((float)pow(2.0, (double)bitdepth) - 1.0);
    printf("Scaling factor is %f\n", scalefactor);
  } else {
    scalefactor = 1.0;
  }

  maxval = ((unsigned int)1 << targetbitdepth) - 1;

  if(channels != targetchannels)
    printf("Expanding from %d channels to %d channels\n", channels, targetchannels);

  // Process each pixel in a single pass, handling both bitdepth
  // and channel changes together
  npixels = width * height;
  for(pixel = 0; pixel < npixels; pixel++){
    png_byte *in_pixel = input + pixel * channels * bytedepth;
    png_byte *out_pixel = output + pixel * targetchannels * targetbytedepth;
    int ch;

    for(ch = 0; ch < targetchannels; ch++){
      unsigned int value;
      int src_ch = map_channel(ch, channels, targetchannels);

      if(src_ch >= 0){
	value = read_sample(in_pixel + src_ch * bytedepth, bytedepth);
	value = (unsigned int)(value * scalefactor + 0.5);
      } else {
	value = maxval;
      }

      if(value > maxval)
	value = maxval;

      write_sample(out_pixel + ch * targetbytedepth, targetbytedepth, value);
    }
  }

  return output;
}
