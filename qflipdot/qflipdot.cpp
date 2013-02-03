/*
 * This file is part of qFlipDot.
 *
 * qFlipDot is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * qFlipDot is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * qFlipDot. If not, see <http://www.gnu.org/licenses/>.
 */
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

