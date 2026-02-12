#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "chunk_meanings.h"

int
main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;
  char s[200];

  while (fgets(s, 200, stdin) != NULL)
    {
      s[strlen(s) - 1] = '\0';
      printf("%s: %s, %s, %s, %s\n", s, isupper(s[0]) ? chunk_meanings[0][0] : chunk_meanings[0][1],
             isupper(s[1]) ? chunk_meanings[1][0] : chunk_meanings[1][1],
             isupper(s[2]) ? chunk_meanings[2][0] : chunk_meanings[2][1],
             isupper(s[3]) ? chunk_meanings[3][0] : chunk_meanings[3][1]);
    }
}
