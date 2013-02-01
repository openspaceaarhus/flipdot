#ifndef __FLIPDOT_QFLIPDOT_H
#define __FLIPDOT_QFLIPDOT_H

#include <stdint.h>

class QFlipDot : public QWidget
{
public:
	QFlipDot(int _w, int _h);
	~QFlipDot();

protected:
	void paintEvent(QPaintEvent *e);

public:
	int setPixel(int x, int y, int clr, bool doupdate = true);
	void clear(void);
	int getWidth()	{ return _width; }
	int getHeight()	{ return _height; }

private:
	int	_width;
	int	_height;
	uint8_t	*screen;
};

#endif
