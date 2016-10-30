#ifndef PBMDRAW_H_
#define PBMDRAW_H_

/// https://en.wikipedia.org/wiki/Netpbm_format
template <typename FlipDotDisplay> class PbmDraw {

public:
  enum class Type : char { P1, P4 };
  PbmDraw(FlipDotDisplay &flipdot, File &file) : type(Type::P1), w(0), h(0), flipdot(flipdot), file(file) {

  }

  bool readHeader() {
    auto line = file.readStringUntil('\n');
    if ('P' != line[0])
      return false;
    if ('1' == line[1])
      type = Type::P1;
    else if ('4' == line[1])
      type = Type::P4;
    else
      return false;

    // skip comments
    while ('#' == line[0]) {
      line = file.readStringUntil('\n');
    }
    line = file.readStringUntil('\n');
    w = file.parseInt();
    h = file.parseInt();
    file.read();                // single whitespace often '\n'
    return true;
  }

  bool blit(int x, int y) {
    if (Type::P4 == type) {
      return blitP4(x,y);
    } else {
      return blitP1(x,y);
    }
  }
  int w, h;
private:

  bool blitP4(int x, int y) {
    int bit = 8;
    char byte = '\0';
    for (int j = 0; j < h; j++) {
      if (bit != 0) bit = 8;    // ignore partial read bytes on row change
      for (int i = 0; i < w; i++) {
        int idx = j * w + i;
        if (8 == bit) {
          byte = file.read();
          bit = 0;
          if (-1 == byte) return false;
        }
        flipdot.drawPixel(i + x, j + y, (byte >> (7-bit)) & 0x1);
        bit++;
      }
    }
    return true;
  }

  bool blitP1(int x, int y) {

    for (int j = 0; j < h; j++) {
      for (int i = 0; i < w; i++) {
        int idx = j * w + i;
        char c = 0;
        do {                    // skip whitespace
          c = file.read();
          if (-1 == c) return false;
        } while (('\n' == c) || (c == ' '));

        flipdot.drawPixel(i + x, j + y, c == '1');
      }
    }
    return true;
  }

  Type type;
  File &file;
  FlipDotDisplay &flipdot;
};
#endif /* !PBMDRAW_H_ */
