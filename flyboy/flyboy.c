#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/time.h>
#include <math.h>

#define XSIZE	20
#define YSIZE	120

#define MAXEXPLODE	25

#define M_2PI	(2.0*M_PI)
#define RAD(d)	((d)*M_PI/180.0)
#define DEG(r)	((r)*180.0/M_PI)

#define F1	17.0
#define F2	11.0

char backstore[XSIZE][YSIZE];
char frontstore[XSIZE][YSIZE];

volatile int quit = 0;
int pfd[0];
struct termios orig_termios;
int posx = XSIZE/2;
int posy = YSIZE/2;
int explode;

#define BWIDTH	5
#define BHEIGHT	10

const uint8_t boat[BWIDTH*BHEIGHT] = {
	0, 0, 1, 0, 0,
	0, 1, 1, 1, 0,
	1, 1, 1, 1, 1,
	1, 1, 1, 1, 1,
	0, 1, 2, 1, 0,
	0, 1, 2, 1, 0,
	0, 1, 2, 1, 0,
	0, 0, 1, 0, 0,
	1, 1, 1, 1, 1,
	0, 0, 1, 0, 0,
};

#define sfpix(x,y,v)	do { \
				if((x) >= 0 && (x) < XSIZE && (y) >= 0 && (y) < YSIZE) \
					frontstore[(x)][(y)] = (v); \
			} while(0);
#define iscol(x,y)	((x) >= 0 && (x) < XSIZE && (y) >= 0 && (y) < YSIZE && frontstore[(x)][(y)])

#define NSHOT	25

int shotx[NSHOT];
int shoty[NSHOT];
double phase1;
double phase2;


int serial_open(const char *port)
{
	struct termios term_opt;
	int fd = open(port, O_RDWR);

	if(fd < 0) {
		perror("failed to open serial port");
		return -1;
	}

	tcgetattr(fd, &term_opt);
	cfmakeraw(&term_opt);
	cfsetispeed(&term_opt, B38400);
	cfsetospeed(&term_opt, B38400);
	tcsetattr(fd, TCSAFLUSH, &term_opt);
	return fd;
}

ssize_t xwrite(int fd, const void *p, size_t n)
{
	ssize_t x;
	size_t s = n;
	while(n) {
		if(-1 == (x = write(fd, p, n))) {
			if(errno == EINTR)
				continue;
			return -1;
		}
		if(!x)
			return s - n;
		n -= x;
		p += x;
	}
	return s;
}

ssize_t xread(int fd, void *p, size_t n)
{
	ssize_t x;
	size_t s = n;
	while(n) {
		if(-1 == (x = read(fd, p, n))) {
			if(errno == EINTR)
				continue;
		}
		if(!x)
			return s - n;
		n -= x;
		p += x;
	}
	return s;
}

void timer_start(void)
{
	struct itimerval itv;
	itv.it_interval.tv_sec = itv.it_value.tv_sec = 0;
	itv.it_interval.tv_usec = itv.it_value.tv_usec = 75000;
	setitimer(ITIMER_REAL, &itv, NULL);
}

void timer_stop(void)
{
	struct itimerval itv;
	memset(&itv, 0, sizeof(itv));
	setitimer(ITIMER_REAL, &itv, NULL);
}

void scr_clear(int fd, int tmr)
{
	int x, y;

	memset(backstore, 0, sizeof(backstore));

	if(tmr)
		timer_stop();
	for(y = 0; y < YSIZE; y++) {
		for(x = 0; x < XSIZE; x++) {
			uint8_t buf[2];
			buf[0] = x;
			buf[1] = y;
			xwrite(fd, buf, 2);
		}
	}
	if(tmr)
		timer_start();
}

void scr_pixel(int fd, int x, int y, int clr)
{
	clr = clr ? 1 : 0;
	if(x >= 0 && x < XSIZE && y >= 0 && y < YSIZE) {
		if(backstore[x][y] ^ clr) {
			uint8_t buf[2];
			buf[0] = x | (clr ? 0x80 : 0);
			buf[1] = y;
			xwrite(fd, buf, 2);
			backstore[x][y] = clr;
		}
	}
}

void scr_frontmap(int fd)
{
	int x, y;

	for(y = 0; y < YSIZE; y++) {
		for(x = 0; x < XSIZE; x++) {
			scr_pixel(fd, x, y, frontstore[x][y]);
		}
	}
}

static const char usage_str[] =
	"Usage: flyboy [-h] <ttyDevice>\n"
	;

static void sighandler(int sig)
{
	(void)sig;
	quit = 1;
}

static void sigalrm(int sig)
{
	char c = 0;
	(void)sig;
	xwrite(pfd[1], &c, 1);
}


void add_shot(void)
{
	int i;
	for(i = 0; i < NSHOT; i++) {
		if(shotx[i] < 0) {
			shotx[i] = posx;
			shoty[i] = posy;
			return;
		}
	}
}

void put_shots(void)
{
	int i;
	for(i = 0; i < NSHOT; i++) {
		if(shotx[i] > 0) {
			sfpix(shotx[i], shoty[i], 1);
			sfpix(shotx[i], shoty[i]-1, 1);
			if(--shoty[i] < 0) {
				shotx[i] = shoty[i] = -1;
			}
		}
	}
}

int put_boat(void)
{
	int x, y;
	int col = 0;
	for(x = 0; x < BWIDTH; x++) {
		for(y = 0; y < BHEIGHT; y++) {
			int px = x + posx - BWIDTH/2;
			int py = y + posy - BHEIGHT/2;
			if(boat[y*BWIDTH + x])
				col |= iscol(px, py);
			sfpix(px, py, boat[y*BWIDTH + x] & 1);
		}
	}
	return col;
}

void put_border(void)
{
	double factor;
	int x, y;

	factor = sin(phase1+phase2);
	factor *= factor;
	factor *= 15.0;
	factor += 85.0;
	factor /= 100.0;
	for(y = 0; y < YSIZE; y++) {
		double p = round(factor * (double)(XSIZE/2)/3.0 * (1.0 + sin(F1*y/(double)YSIZE + phase1)));
		for(x = 0; x < p; x++)
			sfpix(x, y, 1);
		p = round((1.0 - (factor - 1.0)) * (double)(XSIZE/2)/3.0 * (1.0 + sin(F2*y/(double)YSIZE + phase2)));
		for(x = 0; x < p; x++)
			sfpix(XSIZE - x - 1, y, 1);
	}
}

void put_explode(int n)
{
	int i;
	while(n--) {
		for(i = 0; i < 30; i++) {
			int px = posx + n * sin(RAD((double)i*360.0/30.0));
			int py = posy + n * cos(RAD((double)i*360.0/30.0));
			sfpix(px, py, n & 1);
		}
	}
}

void active_step(void)
{
	phase1 -= RAD(1.5);
	phase2 -= RAD(6.0);
	phase1 = fmod(phase1, M_2PI);
	phase2 = fmod(phase2, M_2PI);
}

void reset_game(void)
{
	int i;
	phase1 = phase2 = 0.0;
	posx = XSIZE/2;
	posy = YSIZE/2;
	explode = 0;
	for(i = 0; i < NSHOT; i++) {
		shotx[i] = shoty[i] = -1;
	}
}

void reset_tty(void)
{
	tcsetattr(0, TCSAFLUSH, &orig_termios);
}

int main(int argc, char *argv[])
{
	int fd;
	int optc;
	int lose = 0;
	char *serial_port = NULL;
	int col = 0;

	signal(SIGINT, sighandler);
	signal(SIGQUIT, sighandler);
	signal(SIGHUP, sighandler);
	signal(SIGALRM, sigalrm);

	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	while(EOF != (optc = getopt(argc, argv, "h"))) {
		switch(optc) {
		case 'h':
			printf("%s", usage_str);
			lose++;
		default:
			break;
			lose++;
			break;
		}
	}

	if(lose)
		exit(1);

	if(optind >= argc) {
		fprintf(stderr, "Missing serial port\n");
		exit(1);
	} else {
		serial_port = strdup(argv[optind]);
	}

	if(-1 == pipe(pfd)) {
		perror("pipe");
		exit(1);
	}

	if((fd = serial_open(serial_port)) < 0)
		exit(1);

	if(tcgetattr(0,&orig_termios) < 0) {
		perror("tcgetattr terminal");
		exit(1);
	}

	if(isatty(0)) {
		struct termios raw;

		raw = orig_termios;  /* copy original and then modify below */

		/* input modes - clear indicated ones giving: no break, no CR to NL, 
		no parity check, no strip char, no start/stop output (sic) control */
		raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

		/* output modes - clear giving: no post processing such as NL to CR+NL */
		raw.c_oflag &= ~(OPOST);
		raw.c_oflag |= ONLCR;	/* add CR to NL */

		/* control modes - set 8 bit chars */
		raw.c_cflag |= (CS8);

		/* local modes - clear giving: echoing off, canonical off (no erase with 
		backspace, ^U,...),  no extended functions, no signal chars (^Z,^C) */
		raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

		/* control chars - set return condition: min number of bytes and timer */
		raw.c_cc[VMIN] = 5; raw.c_cc[VTIME] = 8; /* after 5 bytes or .8 seconds
							after first byte seen      */
		raw.c_cc[VMIN] = 0; raw.c_cc[VTIME] = 0; /* immediate - anything       */
		raw.c_cc[VMIN] = 2; raw.c_cc[VTIME] = 0; /* after two bytes, no timer  */
		raw.c_cc[VMIN] = 0; raw.c_cc[VTIME] = 8; /* after a byte or .8 seconds */

		/* put terminal in raw mode after flushing */
		if(tcsetattr(0,TCSAFLUSH,&raw) < 0) {
			perror("tcsetattr raw mode");
			exit(1);
		}
		atexit(reset_tty);
	}

	scr_clear(fd, 0);
	timer_start();
	reset_game();

	while(!quit) {
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(0, &fds);
		FD_SET(pfd[0], &fds);
		if(-1 == select(pfd[0] + 1, &fds, NULL, NULL, NULL)) {
			if(errno == EINTR)
				continue;
			perror("select");
			exit(1);
		}
		if(FD_ISSET(0, &fds)) {
			char c;
			xread(0, &c, 1);
/*
 * up		ESC [ A
 * down		ESC [ B
 * right	ESC [ C
 * left		ESC [ D
 * LT		t
 * RT		z
 * LB		g
 * RB		x
 * TT		5
 * BB		6
 * LL		7
 * RR		8
 */
			switch(c) {
			case 0x1b:
				xread(0, &c, 1);
				if(c == '[') {
					xread(0, &c, 1);
					switch(c) {
					case 'A':
						if(posy > BHEIGHT/2)
							posy--;
						printf("up\r\n");
						break;
					case 'B':
						if(posy < YSIZE-1 - (BHEIGHT+2))
							posy++;
						printf("down\r\n");
						break;
					case 'D':
						if(posx < XSIZE-1-BWIDTH/2)
							posx++;
						printf("right\r\n");
						break;
					case 'C':
						if(posx > BWIDTH/2)
							posx--;
						printf("left\r\n");
						break;
					}
				}
				break;
			case 't':
				add_shot();
				printf("LT\r\n");
				break;
			case 'z':
				printf("RT\r\n");
				break;
			case 'g':
				printf("LB\r\n");
				break;
			case 'x':
				printf("RB\r\n");
				break;
			case '5':
				printf("TT\r\n");
				break;
			case '6':
				printf("BB\r\n");
				break;
			case '7':
				printf("LL\r\n");
				break;
			case '8':
				printf("RR\r\n");
				break;
				break;
			case 'Q':
				quit = 1;
				break;
			}
		}
		if(FD_ISSET(pfd[0], &fds)) {
			char c;
			xread(pfd[0], &c, 1);
			memset(frontstore, 0, sizeof(frontstore));
			if(col) {
				if(explode >= MAXEXPLODE) {
					scr_clear(fd, 1);
					reset_game();
					col = 0;
				} else {
					put_border();
					put_shots();
					put_explode(explode++);
				}
			} else {
				put_border();
				col = put_boat();
				put_shots();
				active_step();
			}
			scr_frontmap(fd);
		}
	}

	timer_stop();
#if 0
	for(i = 0; !quit && i < 1000000; i++) {
		scr_pixel(fd, rand() % XSIZE, rand() % YSIZE, rand() & 1);
	}
#endif

	close(fd);
	return 0;
}
