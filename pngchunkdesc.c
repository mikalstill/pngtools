/******************************************************************************
DOCBOOK START

FUNCTION pngchunkdesc
PURPOSE decode information embedded into a PNG chunk name

SYNOPSIS START
pngchunkdesc
SYNOPSIS END

DESCRIPTION START
PNG files are based around a series of chunks, which embody the information stored in the image file. These chunks have four character ASCII names, where the case of each letter stores additional information.
</para>

<para>
This program decodes the case information in the chunk names, and displays it. The program reads chunk names from stdin, and write chunk descriptions to stdout.
DESCRIPTION END
SEEALSO libpng libtiff tiffinfo pnginfo
DOCBOOK END
******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "chunk_meanings.h"

int
main (int argc, char *argv[])
{
  (void)argc;
  (void)argv;
  char s[200];

  while (fgets (s, 200, stdin) != NULL)
    {
      s[strlen (s) - 1] = '\0';
      printf ("%s: %s, %s, %s, %s\n",
	      s,
	      isupper (s[0]) ? chunk_meanings[0][0] : chunk_meanings[0][1],
	      isupper (s[1]) ? chunk_meanings[1][0] : chunk_meanings[1][1],
	      isupper (s[2]) ? chunk_meanings[2][0] : chunk_meanings[2][1],
	      isupper (s[3]) ? chunk_meanings[3][0] : chunk_meanings[3][1]);
    }
}
