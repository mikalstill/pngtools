// Decode a PNG chunk and tells us stuff about it...

#include <stdio.h>

char *meanings[4][2] = {
  {"Critical", "Ancillary"},
    {"public", "private"},
      {"PNG 1.2 compliant", "in reserved chunk space"},
	{"unsafe to copy", "safe to copy"}};

int main(int argc, char *argv[]){
  char s[200];

  while(fgets(s, 200, stdin) != NULL){
    s[strlen(s) - 1] = '\0';
    printf("%s: %s, %s, %s, %s\n", 
	   s,
	   isupper(s[0]) ? meanings[0][0] : meanings[0][1],
	   isupper(s[1]) ? meanings[1][0] : meanings[1][1],
	   isupper(s[2]) ? meanings[2][0] : meanings[2][1],
	   isupper(s[3]) ? meanings[3][0] : meanings[3][1]);
  }
}
