// List the chunks which appear in a given PNG image

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <arpa/inet.h>
#include "chunk_meanings.h"

static void usage(void);

const char magic[] = { 137, 'P', 'N', 'G', '\r', '\n', 26, '\n' };
typedef struct pngchunks_internal_header
{
  int32_t len;
  union
  {
    int32_t i;
    char c[4];
  } type;
} pngchunks_header;

typedef struct pngchunks_internal_IHDR
{
  int32_t width;
  int32_t height;
  unsigned char bitdepth;
  unsigned char colortype;
  unsigned char compression;
  unsigned char filter;
  unsigned char interlace;
} pngchunks_IHDR;

int
main(int argc, char *argv[])
{
  char *data, *offset;
  int fd, lastchunk;
  struct stat stat;

  if (argc != 2)
    usage();

  if ((fd = open(argv[1], O_RDONLY)) < 0)
    {
      fprintf(stderr, "Could not open the input PNG file\n");
      exit(1);
    }

  if (fstat(fd, &stat) < 0)
    {
      fprintf(stderr, "Could not determine file size\n");
      close(fd);
      exit(1);
    }

  if ((data = mmap(NULL, stat.st_size, PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED)
    {
      fprintf(stderr, "Could not mmap data file\n");
      exit(1);
    }

  offset = data;

  // Check that the file is a PNG file
  if (memcmp(magic, offset, 8) != 0)
    {
      fprintf(stderr, "This is not a PNG file...\n");
      exit(1);
    }
  offset += 8;

  // Go into a loop reading chunks from memory until we hit the end chunk
  lastchunk = 0;
  while (!lastchunk)
    {
      pngchunks_header *head = (pngchunks_header *)offset;
      printf("Chunk: Data Length %u (max %u), Type %d [%c%c%c%c]\n", ntohl(head->len),
             (unsigned int)pow(2, 31) - 1, head->type.i, head->type.c[0], head->type.c[1],
             head->type.c[2], head->type.c[3]);
      offset += sizeof(pngchunks_header);

      printf("  %s, %s, %s, %s\n",
             isupper(head->type.c[0]) ? chunk_meanings[0][0] : chunk_meanings[0][1],
             isupper(head->type.c[1]) ? chunk_meanings[1][0] : chunk_meanings[1][1],
             isupper(head->type.c[2]) ? chunk_meanings[2][0] : chunk_meanings[2][1],
             isupper(head->type.c[3]) ? chunk_meanings[3][0] : chunk_meanings[3][1]);

      if (strncmp(head->type.c, "IHDR", 4) == 0)
        {
          printf("  IHDR Width: %d\n  IHDR Height: %d\n  IHDR Bitdepth: %d\n  IHDR Colortype: %d\n "
                 " IHDR Compression: %d\n  IHDR Filter: %d\n  IHDR Interlace: %d\n",
                 ntohl(((pngchunks_IHDR *)offset)->width),
                 ntohl(((pngchunks_IHDR *)offset)->height), ((pngchunks_IHDR *)offset)->bitdepth,
                 ((pngchunks_IHDR *)offset)->colortype, ((pngchunks_IHDR *)offset)->compression,
                 ((pngchunks_IHDR *)offset)->filter, ((pngchunks_IHDR *)offset)->interlace);

          if (((pngchunks_IHDR *)offset)->compression == 0)
            {
              printf("  IHDR Compression algorithm is Deflate\n");
            }
          else
            {
              printf("  IHDR Compression algorithm is unknown\n");
            }

          if (((pngchunks_IHDR *)offset)->filter == 0)
            {
              printf("  IHDR Filter method is type zero (None, Sub, Up, Average, Paeth)\n");
            }
          else
            {
              printf("  IHDR Filter method is unknown\n");
            }

          switch (((pngchunks_IHDR *)offset)->interlace)
            {
            case 0:
              printf("  IHDR Interlacing is disabled\n");
              break;

            case 7:
              printf("  IHDR Interlacing is Adam7\n");
              break;

            default:
              printf("  IHDR Interlacing method is unknown\n");
              break;
            }
        }
      else if (strncmp(head->type.c, "IDAT", 4) == 0)
        {
          printf("  IDAT contains image data\n");
        }
      else if (strncmp(head->type.c, "IEND", 4) == 0)
        {
          printf("  IEND contains no data\n");
          lastchunk = 1;
        }
      else
        {
          printf("  ... Unknown chunk type\n");
        }

      offset += ntohl(head->len);
      printf("  Chunk CRC: %u\n", ntohl(*((uint32_t *)offset)));
      offset += 4;
    }

  // Unmap the file
  if (munmap(data, stat.st_size) < 0)
    {
      fprintf(stderr, "Error unmapping memory\n");
      exit(1);
    }
}

static void
usage(void)
{
  fprintf(stderr, "Usage: pngchunks <filename>\n");
  exit(1);
}
