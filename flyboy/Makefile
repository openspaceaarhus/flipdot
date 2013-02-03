
CFLAGS=-Wall -O2 -g
LIBS=-lm

SRCS=\
	flyboy.c

OBJS=$(SRCS:.c=.o)


flyboy: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $+ $(LIBS)

flyboy.o: flyboy.c

clean::
	$(RM) flyboy $(OBJS)
