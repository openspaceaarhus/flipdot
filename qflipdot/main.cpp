#include <QApplication>

#include "fddialog.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	FlipdotDialog dialog;
	return dialog.exec();
}
