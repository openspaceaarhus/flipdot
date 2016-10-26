#define TEST

#include "PbmDraw.h"
#include "TerminalFake.h"
#include "maze.cpp"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int main(int argc, char *argv[]) {

  if (argc < 1) {
    fprintf(stderr, "usage ./%s file\n", argv[0]);
    return -1;
  }

  TerminalFake fake;
  PbmDraw<TerminalFake> pbm(fake);

  FILE *f = fopen(argv[1], "r");
  size_t len = 0;
  int s = 13;
  char buf[s];

  while (0 != (len = fread(buf, sizeof(char), s, f))) {
    pbm.read(buf, len);
  }

  if (!pbm.parse()) {
    printf("PARSE ERROR\n");
    return -1;
  }
  pbm.blit(0, 0);
  fake.display();

  return 0;
}
