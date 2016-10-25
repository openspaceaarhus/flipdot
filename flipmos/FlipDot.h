#ifndef FLIPDOT_H_
#define FLIPDOT_H_

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <algorithm>
#include <bitset>

#include "Config.h"

template <typename Mirror> class FlipDot : public Adafruit_GFX {
public:
  FlipDot(Mirror &m) : Adafruit_GFX(SIGN_W, SIGN_H), mirror(m) {
    framebuffer.reset();
    currentDisplay.set();
    display();
  };

  void drawPixel(int16_t x, int16_t y, uint16_t color) override {
    int idx = y * width() + x;
    framebuffer[idx] = (color != 0);
  }

  /// send changes to display
  void display() {
    for (int j = 0; j < height(); j++) {
      for (int i = 0; i < width(); i++) {
        int idx = j * width() + i;
        if (framebuffer[idx] != currentDisplay[idx])
          plot(i, j, framebuffer[idx]);
      }
    }
    currentDisplay = framebuffer;
    mirror.display();
  }

  void invert() { framebuffer.flip(); }

  void forceAll(char color) {
    for (int j = 0; j < height(); j++) {
      for (int i = 0; i < width(); i++) {
        sendpixel(i, j, color);
      }
    }
  }

  void reset() {
    for (int i = 0; i < 5; i++)
      forceAll(0);
    for (int i = 0; i < 5; i++)
      forceAll(1);
    framebuffer.reset();
    currentDisplay.set();
    display();
  }

private:
  void sendpixel(char x, char y, char on) {
    /// the display is coded so the x axis is the short axis, but is mounted in
    /// a horisontal
    /// position essentially flipping the axis. For sanity this is the only
    /// place we do the
    /// flippiing.
    Serial.write((y & 0x7F) | on << 7);
    Serial.write(x);
  }
  void plot(char x, char y, char on) {
    sendpixel(x, y, on);
    // wrap (wemos) display to show full screen in a manner
    if (x > mirror.width()) {
      x -= mirror.width();
      y += SIGN_H + 2;
    }
    mirror.drawPixel(x, y, on ? WHITE : BLACK);
  }
  Mirror &mirror;
  std::bitset<SIGN_W * SIGN_H> framebuffer;
  std::bitset<SIGN_W * SIGN_H> currentDisplay;
};

#endif /* !FLIPDOT_H_ */
