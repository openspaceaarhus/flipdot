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
#ifndef __QFLIPDOT_FDDIALOG_H
#define __QFLIPDOT_FDDIALOG_H

#include <QDialog>

class QFlipDot;
class QSocketNotifier;
class QTimer;

class FlipdotDialog : public QDialog
{
	Q_OBJECT

public:
	FlipdotDialog();
	~FlipdotDialog();

private slots:
	void fifoHandler(int fd);
	void throttle();

private:
	void reopen_fifo(void);

	QFlipDot	*flipdot;
	QSocketNotifier	*sn;
	QTimer		*limittimer;
	int		fifofd;
	int		fifostate;
	char		fifoc1;
	int		ppms;
};

#endif
