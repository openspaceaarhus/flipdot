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
#ifndef __QFLIPDOT_QFLIPDOT_H
#define __QFLIPDOT_QFLIPDOT_H

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
