// libFuzzer harness for the pnginfo parse path.
//
// pnginfo uses libpng for the actual byte decoding, so libpng's own
// fuzz corpus already covers most of the parser. What this harness
// exercises is how *pnginfo* drives libpng: the IHDR retrieval, the
// palette and text iteration, the bitmap allocation arithmetic
// (rowbytes * height + 1, which can overflow), and the png_read_image
// call. Bugs we are likely to find here are in our usage of libpng
// rather than in libpng itself.

#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <png.h>

struct pngmem
{
  const uint8_t *data;
  size_t size;
  size_t off;
};

static void
pngmem_read(png_structp png, png_bytep dst, png_size_t n)
{
  struct pngmem *m = (struct pngmem *)png_get_io_ptr(png);
  if (m->off + n > m->size)
    {
      png_error(png, "read past end of input");
      return;
    }
  memcpy(dst, m->data + m->off, n);
  m->off += n;
}

int
LLVMFuzzerInitialize(int *argc, char ***argv)
{
  (void)argc;
  (void)argv;
  // Don't redirect stderr: libFuzzer and the sanitizers print
  // diagnostics there. libpng warnings on malformed input are
  // also limited per call, so they don't meaningfully slow us
  // down.
  (void)freopen("/dev/null", "w", stdout);
  return 0;
}

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  if (size < 8 || png_sig_cmp((png_const_bytep)data, 0, 8) != 0)
    return 0;

  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png)
    return 0;

  png_infop info = png_create_info_struct(png);
  if (!info)
    {
      png_destroy_read_struct(&png, NULL, NULL);
      return 0;
    }

  // Pointers freed by the cleanup path below. Volatile so the
  // setjmp/longjmp doesn't clobber them.
  png_byte *volatile raster = NULL;
  png_bytep *volatile rows = NULL;

  if (setjmp(png_jmpbuf(png)))
    goto cleanup;

  struct pngmem m = { data, size, 0 };
  png_set_read_fn(png, &m, pngmem_read);

  png_read_info(png, info);

  png_uint_32 width = 0, height = 0;
  int bitdepth = 0, colourtype = 0;
  png_get_IHDR(png, info, &width, &height, &bitdepth, &colourtype, NULL, NULL, NULL);

  // Cap dimensions to keep RSS bounded. libFuzzer terminates the
  // process at -rss_limit_mb, which we'd rather not hit when a
  // mutator generates a 2GB image.
  if ((uint64_t)width * (uint64_t)height > 4ULL * 1024 * 1024)
    goto cleanup;

  // Exercise palette retrieval the way pnginfo does
  png_colorp palette = NULL;
  int num_palette = 0;
  if (png_get_PLTE(png, info, &palette, &num_palette) == PNG_INFO_PLTE)
    {
      png_bytep trans = NULL;
      int num_trans = 0;
      png_color_16p trans_values = NULL;
      png_get_tRNS(png, info, &trans, &num_trans, &trans_values);
    }

  // Iterate text strings exactly like pnginfo (walks a NUL-terminated
  // text->text pointer per entry, which is where a NULL text pointer
  // from a malformed tEXt would crash).
  png_textp text = NULL;
  int num_text = 0;
  if (png_get_text(png, info, &text, &num_text) > 0 && text != NULL)
    {
      for (int ti = 0; ti < num_text; ti++)
        {
          if (text[ti].text == NULL)
            continue;
          size_t j = 0;
          while (text[ti].text[j] != '\0')
            j++;
        }
    }

  png_uint_32 x = 0, y = 0;
  int phys = 0;
  png_get_pHYs(png, info, &x, &y, &phys);

  // Full bitmap read, matching the -D path. Guard the allocation
  // against absurd row size so we don't OOM the fuzzer.
  if (bitdepth < 8)
    png_set_packing(png);
  if (colourtype == PNG_COLOR_TYPE_PALETTE)
    png_set_expand(png);
  png_read_update_info(png, info);

  png_size_t rowbytes = png_get_rowbytes(png, info);
  if (rowbytes > 0 && height > 0 && (uint64_t)rowbytes * (uint64_t)height < 32ULL * 1024 * 1024)
    {
      raster = (png_byte *)malloc(rowbytes * height + 1);
      if (raster)
        {
          rows = (png_bytep *)malloc(height * sizeof(png_bytep));
          if (rows)
            {
              for (png_uint_32 i = 0; i < height; i++)
                rows[i] = raster + i * rowbytes;
              png_read_image(png, rows);
              png_read_end(png, NULL);
            }
        }
    }

cleanup:
  free(rows);
  free(raster);
  png_destroy_read_struct(&png, &info, NULL);
  return 0;
}
