#include <QtGui>
#include <QWidget>

#include <string.h>

#include "qflipdot.h"

#define PSIZE	6

QFlipDot::QFlipDot(int _w, int _h)
{
	_width = _w;
	_height = _h;
	setMinimumWidth(PSIZE*_width);
	setMinimumHeight(PSIZE*_height);
	setMaximumWidth(PSIZE*_width);
	setMaximumHeight(PSIZE*_height);
	screen = new uint8_t[_width * _height];
}

QFlipDot::~QFlipDot()
{
	delete[] screen;
}

void QFlipDot::paintEvent(QPaintEvent *e)
{
	int x, y;
	QPainter p(this);
	//p.setRenderHint(QPainter::Antialiasing, true);
	p.setRenderHint(QPainter::SmoothPixmapTransform, true);
	p.fillRect(rect(), QBrush(Qt::black));
	p.setPen(QPen(Qt::white));
	int w = width() / _width;
	int h = height() / _height;
	for(y = 0; y < _height; y++) {
		for(x = 0; x < _width; x++) {
			if(e->rect().intersects(QRect((_width - x - 1)*PSIZE, y*PSIZE, PSIZE, PSIZE))) {
				p.setBrush(QBrush(screen[y * _width + x] ? Qt::white : Qt::black));
				p.drawEllipse(width()*(_width - x - 1)/_width, height()*y/_height, w, h);
			}
		}
	}
}

int QFlipDot::setPixel(int x, int y, int clr, bool doupdate)
{
	int rv = 0;
	clr = clr ? 1 : 0;
	if(x >= 0 && x < _width && y >= 0 && y < _height) {
		if(clr ^ screen[y * _width + x]) {
			rv = 1;
			screen[y * _width + x] = clr ? 1 : 0;
		}
	}
	if(doupdate) {
		update((_width - x - 1)*PSIZE, y*PSIZE, PSIZE, PSIZE);
	}
	return rv;
}

void QFlipDot::clear(void)
{
	memset(screen, 0, _width*_height*sizeof(*screen));
	update();
}

