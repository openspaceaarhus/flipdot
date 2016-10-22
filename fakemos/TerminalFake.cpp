#include "TerminalFake.h"

TerminalFake::TerminalFake() {
  framebuffer.reset();
}
void TerminalFake::drawPixel(int16_t x, int16_t y, uint16_t color) {
  auto idx = x + y * width();
  framebuffer[idx] = (color != 0);
}

int TerminalFake::width() { return SIGN_W; }
int TerminalFake::height() { return SIGN_H; }

void TerminalFake::display() {
  for (int j = 0; j < height(); j++) {
    for (int i = 0; i < width(); i++) {
      auto idx = i + j * width();
      putchar(framebuffer[idx] ? '*' : 'O');
    }
    putchar('\n');
  }
};
