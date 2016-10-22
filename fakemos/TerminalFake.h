#include "Config.h"
#include <bitset>

class TerminalFake {
public:
  TerminalFake();
  void drawPixel(int16_t x, int16_t y, uint16_t color);
  int width();
  int height();
  void display();
private:
  std::bitset<SIGN_W * SIGN_H> framebuffer;
};
