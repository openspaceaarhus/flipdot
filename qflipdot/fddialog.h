#ifndef __FLIPDOT_FDDIALOG_H
#define __FLIPDOT_FDDIALOG_H

#include <QDialog>

class QFlipDot;
class QSocketNotifier;

class FlipdotDialog : public QDialog
{
	Q_OBJECT

public:
	FlipdotDialog();
	~FlipdotDialog();

private slots:
	void fifoHandler(int fd);

private:
	void reopen_fifo(void);

	QFlipDot	*flipdot;
	QSocketNotifier	*sn;
	int		fifofd;
	int		fifostate;
	char		fifoc1;
};

#endif
