// List the chunks which appear in a given PNG image

#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>

void usage();

const char magic[] = {137, 'P', 'N', 'G', '\r', '\n', 26, '\n'};
typedef struct pngchunks_internal_header
{
  // We are assuming here that ints are 4 bytes...
  unsigned int len;
  union{
    unsigned int i;
    char c[4];
  } type;
} pngchunks_header;

char *meanings[4][2] = {
  {"Critical", "Ancillary"},
  {"public", "private"},
  {"PNG 1.2 compliant", "in reserved chunk space"},
  {"unsafe to copy", "safe to copy"}
};

typedef struct pngchunks_internal_IHDR
{
  int width;
  int height;
  unsigned char bitdepth;
  unsigned char colortype;
  unsigned char compression;
  unsigned char filter;
  unsigned char interlace;
} pngchunks_IHDR;

int main(int argc, char *argv[])
{
  char *data, *offset;
  int fd, lastchunk;
  struct stat stat;
  pngchunks_header *head;

  if((sizeof(int) != 4) || (sizeof(long) != 4))
    {
      fprintf(stderr, "Int or long is not the assumed four byte size...\n");
      exit(1);
    }

  if(argc != 2)
    usage();

  if ((fd = open (argv[1], O_RDONLY)) < 0)
    {
      fprintf (stderr, "Could not open the input PNG file\n");
      exit (1);
    }

  if (fstat(fd, &stat) < 0)
    {
      fprintf(stderr, "Could not determine file size\n");
      close(fd);
      exit(1);
    }

  if ((data = mmap (NULL, stat.st_size, PROT_READ, MAP_SHARED, fd, 0)) < 0)
    {
      fprintf (stderr, "Could not mmap data file\n");
      exit (1);
    }

  offset = data;

  // Check that the file is a PNG file
  if(memcmp(magic, offset, 8) != 0){
    fprintf(stderr, "This is not a PNG file...\n");
    exit(1);
  }
  offset += 8;

  // Go into a loop reading chunks from memory until we hit the end chunk
  lastchunk = 0;
  while(!lastchunk)
    {
      head = (pngchunks_header *) offset;
      printf("Chunk: Data Length %d (max %d), Type %d [%c%c%c%c]\n", 
	     ntohl(head->len), 
	     (unsigned int) pow(2, 31) - 1,
	     head->type.i,
	     head->type.c[0], head->type.c[1],
	     head->type.c[2], head->type.c[3]);
      offset += sizeof(pngchunks_header);

      printf ("  %s, %s, %s, %s\n",
              isupper (head->type.c[0]) ? meanings[0][0] : meanings[0][1],
              isupper (head->type.c[1]) ? meanings[1][0] : meanings[1][1],
              isupper (head->type.c[2]) ? meanings[2][0] : meanings[2][1],
              isupper (head->type.c[3]) ? meanings[3][0] : meanings[3][1]);

      if(strncmp(head->type.c, "IHDR", 4) == 0)
	{
	  printf("  IHDR Width: %d\n  IHDR Height: %d\n  IHDR Bitdepth: %d\n  IHDR Colortype: %d\n  IHDR Compression: %d\n  IHDR Filter: %d\n  IHDR Interlace: %d\n",
		 ntohl(((pngchunks_IHDR *) offset)->width),
		 ntohl(((pngchunks_IHDR *) offset)->height),
		 ((pngchunks_IHDR *) offset)->bitdepth,
		 ((pngchunks_IHDR *) offset)->colortype,
		 ((pngchunks_IHDR *) offset)->compression,
		 ((pngchunks_IHDR *) offset)->filter,
		 ((pngchunks_IHDR *) offset)->interlace);

	  switch( ((pngchunks_IHDR *) offset)->compression){
	  case 0:
	    printf("  IHDR Compression algorithm is Deflate\n");
	    break;

	  default:
	    printf("  IHDR Compression algorithm is unknown\n");
	    break;
	  }

	  switch( ((pngchunks_IHDR *) offset)->filter)
	    {
	    case 0:
	      printf("  IHDR Filter method is type zero (None, Sub, Up, Average, Paeth)\n");
	      break;
	      
	    default:
	      printf("  IHDR Filter method is unknown\n");
	      break;
	    }

	  switch( ((pngchunks_IHDR *) offset)->interlace)
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
      else if(strncmp(head->type.c, "IDAT", 4) == 0)
	{
	  printf("  IDAT contains image data\n");
	}
      else if(strncmp(head->type.c, "IEND", 4) == 0)
	{
	  printf("  IEND contains no data\n");
	  lastchunk = 1;
	}
      else{
	printf("  ... Unknown chunk type\n");
      }

      offset += ntohl(head->len);
      printf("  Chunk CRC: %d\n", ntohl(*((long *) offset)));
      offset += 4;
    }

  // Unmap the file
  if(munmap(data, stat.st_size) < 0)
    {
      fprintf(stderr, "Error unmapping memory\n");
      exit(1);
    }
}


void usage()
{
  fprintf(stderr, "Usage: pngchunks <filename>\n");
  exit(1);
}
