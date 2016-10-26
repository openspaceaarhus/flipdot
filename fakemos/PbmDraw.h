#ifndef PBMDRAW_H_
#define PBMDRAW_H_
#include <string>

#include <iostream>
#include <algorithm>

/// https://en.wikipedia.org/wiki/Netpbm_format
template <typename FlipDotDisplay> class PbmDraw {

public:
  enum class Type : char { P1, P4 };
  PbmDraw(FlipDotDisplay &flipdot) : type(Type::P1), w(0), h(0), flipdot(flipdot) {}

  void read(const char *data, int len) {
    buffer.append(data, len);
    printf("Added %d data to buffer %d :\n\t%s\n", len, buffer.size(), buffer.c_str());
  }

  bool parse() {
    if ('P' != buffer[0])
      return false;
    if ('1' == buffer[1])
      type = Type::P1;
    else if ('4' == buffer[1])
      type = Type::P4;
    else
      return false;
    eatLine();

    // skip comments
    while ('#' == buffer[0]) {
      eatLine();
    }

    w = std::stoi(buffer.substr(0, buffer.find(' ')));
    h = std::stoi(buffer.substr(buffer.find(' ')));
    eatLine();
    //    printf("REMAINDER:\n%s\n", buffer.substr(w*h).c_str());
    if (Type::P1 == type) {
      /// remove newlines whitespace etc
      buffer.erase(std::remove_if(buffer.begin(), buffer.end(), [](char x) {
        return !(x == '1' || x == '0');
      }));
    printf("%d x %d in a buffer %d (%d)\n", w, h, buffer.size(), w*h);
    printf("\n%s\n", buffer.c_str());

      return w*h <= buffer.size();
    }
  }

  void blit(int x, int y) {
    printf("BLIT\n");
    printf("%d x %d\n", w, h);

    if (Type::P4 == type) {
      blitP4(x,y);
    } else {
      blitP1(x,y);
    }
  }

private:
  void blitP4(int x, int y) {
    for (int j = 0; j < h; j++) {
      for (int i = 0; i < w; i++) {
        int idx = j * w + i;
        auto byte = buffer[idx / 8];
        auto color = byte >> (idx % 8);
        flipdot.drawPixel(i + x, j + y, color);
      }
    }
  }

  void blitP1(int x, int y) {
    for (int j = 0; j < h; j++) {
      for (int i = 0; i < w; i++) {
        int idx = j * w + i;
        flipdot.drawPixel(i + x, j + y, buffer[idx] == '1');
      }
    }
  }

  Type type;
  void eatLine() { buffer = buffer.substr(buffer.find('\n') + 1); }
  int w, h;
  std::string buffer;
  FlipDotDisplay &flipdot;
};
#endif /* !PBMDRAW_H_ */
