#include <QtGui>
#include <QDialog>
#include <QSocketNotifier>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "qflipdot.h"
#include "fddialog.h"

#define XSIZE	20
#define YSIZE	120

#define FIFOPATH	"./tty_interface"

FlipdotDialog::FlipdotDialog()
{
	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	if(-1 == ::mkfifo(FIFOPATH, 0600)) {
		if(errno != EEXIST) {
			perror("mkfifo");
			exit(1);
		}
	}
	fifofd = ::open(FIFOPATH, O_RDONLY|O_NONBLOCK);
	if(-1 == fifofd) {
		perror("open");
		exit(1);
	}
	sn = new QSocketNotifier(fifofd, QSocketNotifier::Read);
	connect(sn, SIGNAL(activated(int)), this, SLOT(fifoHandler(int)));
	sn->setEnabled(true);

	QVBoxLayout *vbox = new QVBoxLayout();
	flipdot = new QFlipDot(XSIZE, YSIZE);
	vbox->addWidget(flipdot);

	QVBoxLayout *mainLayout = new QVBoxLayout();
	mainLayout->addLayout(vbox);
	mainLayout->addWidget(buttonBox);
	setLayout(mainLayout);

	fifostate = 0;

	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), flipdot, SLOT(update()));
	timer->start(20);
}

FlipdotDialog::~FlipdotDialog()
{
	::close(fifofd);
}

void FlipdotDialog::reopen_fifo(void)
{
	sn->setEnabled(false);
	delete sn;
	::close(fifofd);
	fifofd = ::open(FIFOPATH, O_RDONLY|O_NONBLOCK);
	if(-1 == fifofd) {
		perror("open (reopen)");
		exit(1);
	}
	sn = new QSocketNotifier(fifofd, QSocketNotifier::Read);
	connect(sn, SIGNAL(activated(int)), this, SLOT(fifoHandler(int)));
	sn->setEnabled(true);
}

void FlipdotDialog::fifoHandler(int fd)
{
	ssize_t n;
	char c2;
	sn->setEnabled(false);
retry:
	if(!fifostate) {
		n = ::read(fd, &fifoc1, 1);
		if(-1 == n && errno == EINTR)
			goto retry;
		if(!n) {
			reopen_fifo();
			fifostate = 0;
			return;
		}
		fifostate = 1;
	} else if(fifostate == 1) {
		n = ::read(fd, &c2, 1);
		if(-1 == n && errno == EINTR)
			goto retry;
		if(!n) {
			reopen_fifo();
			fifostate = 0;
			return;
		}
		fifostate = 0;
		flipdot->setPixel(fifoc1 & 0x7f, c2, fifoc1 & 0x80, false);
	} else {
		fifostate = 0;
	}
	sn->setEnabled(true);
}
