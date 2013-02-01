HEADERS = fddialog.h \
	qflipdot.h

SOURCES = fddialog.cpp \
	main.cpp \
	qflipdot.cpp

# install
target.path = .
sources.files = $$SOURCES $$HEADERS *.pro
sources.path = .
INSTALLS += target sources
QT += network
