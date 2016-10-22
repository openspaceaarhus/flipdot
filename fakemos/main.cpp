#include "maze.cpp"
#include "TerminalFake.h"

int main(int argc, char *argv[]) {
  
  TerminalFake fake;

  for (int i = 0; i < SIGN_W; i++) {
    fake.drawPixel(i, 3, 1);
  }
  fake.display();

  return 0;

}
