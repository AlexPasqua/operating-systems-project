#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main (int argc, char *argv[]) {
  char result[200] = "";

  for (int i = 1; i < argc; i++){
    strcat(result, argv[i]);
    strcat(result, " ");
  }

  printf("%s", result);

  return 0;
}
