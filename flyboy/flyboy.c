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

//#define DEBUG
#ifdef DEBUG
#define debug_printf	printf
#else
#define debug_printf(f,...)
#endif

#define XSIZE	20
#define YSIZE	120

#define MAXEXPLODE	35

#define DELAYTIMEMIN	50000
#define DELAYTIMEMAX	250000
#define DELAYTIMESTEP	100
volatile int delaytime = DELAYTIMEMAX;

#define M_2PI	(2.0*M_PI)
#define RAD(d)	((d)*M_PI/180.0)
#define DEG(r)	((r)*180.0/M_PI)

#define F1	2.22
#define F2	3.33

char backstore[YSIZE][XSIZE];
char frontstore[YSIZE][XSIZE];
char wavestore[YSIZE][XSIZE];

volatile int quit = 0;
int pfd[0];
struct termios orig_termios;
int posx = XSIZE/2;
int posy = YSIZE/2;
int explode;
int wavey;
int resetstate;
int explodestate;

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
					frontstore[(y)][(x)] = (v); \
			} while(0);
#define swpix(x,y,v)	do { \
				if((x) >= 0 && (x) < XSIZE && (y) >= 0 && (y) < YSIZE) \
					wavestore[(y)][(x)] = (v); \
			} while(0);
#define iscol(x,y)	((x) >= 0 && (x) < XSIZE && (y) >= 0 && (y) < YSIZE && frontstore[(y)][(x)])

#define NSHOT	25

int shotx[NSHOT];
int shoty[NSHOT];
double phase1;
double phase2;
double phoffs1;
double phoffs2;


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
	itv.it_interval.tv_sec = 0;
	itv.it_interval.tv_usec = 0;
	itv.it_value.tv_sec = 0;
	itv.it_value.tv_usec = delaytime;
	setitimer(ITIMER_REAL, &itv, NULL);
	if(delaytime > DELAYTIMEMIN)
		delaytime -= DELAYTIMESTEP;
}

void timer_stop(void)
{
	struct itimerval itv;
	memset(&itv, 0, sizeof(itv));
	setitimer(ITIMER_REAL, &itv, NULL);
}

void scr_pixel(int fd, int x, int y, int clr)
{
	clr = clr ? 1 : 0;
	if(x >= 0 && x < XSIZE && y >= 0 && y < YSIZE) {
		if(backstore[y][x] ^ clr) {
			uint8_t buf[2];
			buf[0] = x | (clr ? 0x80 : 0);
			buf[1] = y;
			xwrite(fd, buf, 2);
			backstore[y][x] = clr;
		}
	}
}

void scr_frontmap(int fd)
{
	int x, y;

	for(y = 0; y < YSIZE; y++) {
		for(x = 0; x < XSIZE; x++) {
			scr_pixel(fd, x, y, frontstore[y][x]);
		}
	}
}

/* Bresenham line algorithm (from wikipedia) */
void bh_line(int x0, int y0, int x1, int y1, int rf, int clr)
{
	int i;
	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);
	int sx = x0 < x1 ? 1 : -1;
	int sy = y0 < y1 ? 1 : -1;
	int err = dx - dy;
	int e2;

	while(1) {
		if(x0 <= -XSIZE || x0 >= 2*XSIZE || x1 <= -XSIZE || x1 >= 2*XSIZE) {
			printf("X-coord out of range? (%d,%d) (%d,%d)\r\n", x0, y0, x1, y1);
			return;
		}
		if(y0 <= -YSIZE || y0 >= 2*YSIZE || y1 <= -YSIZE || y1 >= 2*YSIZE) {
			printf("Y-coord out of range? (%d,%d) (%d,%d)\r\n", x0, y0, x1, y1);
			return;
		}
		/* setPixel(x0,y0) */
		if(rf) {
			for(i = x0; i < XSIZE; i++)
				sfpix(i, y0, clr);
		} else {
			for(i = x0; i >= 0; i--)
				sfpix(i, y0, clr);
		}
		if(x0 == x1 && y0 == y1)
			break;
		e2 = 2*err;
		if(e2 > -dy) {
			err -= dy;
			x0 += sx;
		}
		if(e2 < dx) {
			err += dx;
			y0 += sy;
		}
	}
}

void animate_clear(int fd)
{
	int x, y;
	for(y = 0; y < YSIZE/2; y++) {
		bh_line(XSIZE/2, YSIZE/2, XSIZE-1, YSIZE/2+y-1, 1, 1);
		bh_line(XSIZE/2, YSIZE/2, 0,       YSIZE/2+y-1, 0, 1);
		bh_line(XSIZE/2, YSIZE/2, XSIZE-1, YSIZE/2-y,   1, 1);
		bh_line(XSIZE/2, YSIZE/2, 0,       YSIZE/2-y,   0, 1);
		scr_frontmap(fd);
	}
	for(x = 0; x < XSIZE/2; x++) {
		bh_line(XSIZE/2, YSIZE/2, XSIZE-x-1, YSIZE-1, 1, 1);
		bh_line(XSIZE/2, YSIZE/2, XSIZE-x-1, 0,       1, 1);
		bh_line(XSIZE/2, YSIZE/2, x,         YSIZE-1, 0, 1);
		bh_line(XSIZE/2, YSIZE/2, x,         0,       0, 1);
		scr_frontmap(fd);
	}

	for(y = 0; y < YSIZE/2; y++) {
		bh_line(XSIZE/2, YSIZE/2, XSIZE-1, YSIZE/2+y-1, 1, 0);
		bh_line(XSIZE/2, YSIZE/2, 0,       YSIZE/2+y-1, 0, 0);
		bh_line(XSIZE/2, YSIZE/2, XSIZE-1, YSIZE/2-y,   1, 0);
		bh_line(XSIZE/2, YSIZE/2, 0,       YSIZE/2-y,   0, 0);
		scr_frontmap(fd);
	}
	for(x = 0; x < XSIZE/2; x++) {
		bh_line(XSIZE/2, YSIZE/2, XSIZE-x-1, YSIZE-1, 1, 0);
		bh_line(XSIZE/2, YSIZE/2, XSIZE-x-1, 0,       1, 0);
		bh_line(XSIZE/2, YSIZE/2, x,         YSIZE-1, 0, 0);
		bh_line(XSIZE/2, YSIZE/2, x,         0,       0, 0);
		scr_frontmap(fd);
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
			shoty[i] = posy - BHEIGHT/2;
			return;
		}
	}
}

void put_shots(int animate)
{
	int i;
	for(i = 0; i < NSHOT; i++) {
		if(shotx[i] > 0) {
			sfpix(shotx[i], shoty[i], 1);
			sfpix(shotx[i], shoty[i]+1, 1);
			if(animate) {
				if(--shoty[i] < 0) {
					shotx[i] = shoty[i] = -1;
				}
			}
		}
	}
}

void shoot_wave(void)
{
	int i;
	for(i = 0; i < NSHOT; i++) {
		if(shotx[i] > 0) {
			if(wavestore[shoty[i]][shotx[i]] || wavestore[shoty[i]+1][shotx[i]]) {
#if 0
				swpix(shotx[i]-1, shoty[i]-2, 0);
				swpix(shotx[i]+0, shoty[i]-2, 0);
				swpix(shotx[i]+1, shoty[i]-2, 0);
				swpix(shotx[i]-2, shoty[i]-1, 0);
				swpix(shotx[i]-1, shoty[i]-1, 0);
				swpix(shotx[i]+0, shoty[i]-1, 0);
				swpix(shotx[i]+1, shoty[i]-1, 0);
				swpix(shotx[i]+2, shoty[i]-1, 0);
				swpix(shotx[i]-3, shoty[i]+0, 0);
				swpix(shotx[i]-2, shoty[i]+0, 0);
				swpix(shotx[i]-1, shoty[i]+0, 0);
				swpix(shotx[i]+0, shoty[i]+0, 0);
				swpix(shotx[i]+1, shoty[i]+0, 0);
				swpix(shotx[i]+2, shoty[i]+0, 0);
				swpix(shotx[i]+3, shoty[i]+0, 0);
				swpix(shotx[i]-2, shoty[i]+1, 0);
				swpix(shotx[i]-1, shoty[i]+1, 0);
				swpix(shotx[i]+0, shoty[i]+1, 0);
				swpix(shotx[i]+1, shoty[i]+1, 0);
				swpix(shotx[i]+2, shoty[i]+1, 0);
				swpix(shotx[i]-1, shoty[i]+2, 0);
				swpix(shotx[i]+0, shoty[i]+2, 0);
				swpix(shotx[i]+1, shoty[i]+2, 0);
#else
				int x, y;
				for(x = -2; x <= 2; x++) {
					for(y = -2; y <= 2; y++) {
						swpix(shotx[i]+x, shoty[i]+y, 0);
					}
				}
#endif
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

void put_border(int move)
{
	double factor;
	double p;
	int x;

	if(move) {
		memmove(&wavestore[1][0], &wavestore[0][0], sizeof(wavestore) - XSIZE*sizeof(wavestore[0][0]));
		memset(&wavestore[0][0], 0, XSIZE*sizeof(wavestore[0][0]));

		factor = sin(phase1+phase2);
		factor *= factor;
		factor *= 15.0;
		factor += 85.0;
		factor /= 100.0;
#define BUMPSIZE	((rand() % 10000) < 100 ? 2.5 : 2.7)
		p = round(factor * (double)(XSIZE/2)/BUMPSIZE * (1.0 + sin(phase1)));
		for(x = 0; x < p; x++)
			wavestore[0][x] = 1;
		p = round((1.0 - (factor - 1.0)) * (double)(XSIZE/2)/BUMPSIZE * (1.0 + sin(phase2)));
		for(x = 0; x < p; x++)
			wavestore[0][XSIZE - x - 1] = 1;
	}

	memcpy(frontstore, wavestore, sizeof(wavestore));
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
	phase1 = M_2PI*F1*(double)wavey/(double)YSIZE - RAD(1.5) + phoffs1;
	phase2 = M_2PI*F2*(double)wavey/(double)YSIZE - RAD(6.0) + phoffs2;
	wavey = (wavey + 1) % YSIZE;
	if(phase1 <= -M_2PI)
		phase1 += M_2PI;
	else if(phase1 >= M_2PI)
		phase1 -= M_2PI;
	if(phase2 <= -M_2PI)
		phase2 += M_2PI;
	else if(phase2 >= M_2PI)
		phase2 -= M_2PI;
}

void reset_game(void)
{
	int i;
	phoffs1 = M_2PI * (double)(rand() % 10000)/10000.0;
	phoffs2 = M_2PI * (double)(rand() % 10000)/10000.0;
	posx = XSIZE/2-1;
	posy = 3*YSIZE/4;
	explode = 0;
	wavey = 0;
	resetstate = 0;
	explodestate = 0;
	delaytime = DELAYTIMEMAX;
	for(i = 0; i < NSHOT; i++) {
		shotx[i] = shoty[i] = -1;
	}
	memset(wavestore, 0, sizeof(wavestore));
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

	/* save the original terminal setup to restore at exit */
	if(tcgetattr(0,&orig_termios) < 0) {
		perror("tcgetattr terminal");
		exit(1);
	}

	if(isatty(0)) {
		struct termios raw;

		raw = orig_termios;
		cfmakeraw(&raw);
		raw.c_cc[VMIN] = 0;
		raw.c_cc[VTIME] = 0;

		/* put terminal in raw mode after flushing */
		if(tcsetattr(0,TCSAFLUSH,&raw) < 0) {
			perror("tcsetattr raw mode");
			exit(1);
		}
		atexit(reset_tty);
	}

	animate_clear(fd);
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
			ssize_t n = xread(0, &c, 1);
			if(-1 == n) {
				perror("read char");
				exit(1);
			} else if(!n) {
				fprintf(stderr, "Serial port closed on me...\r\n");
				exit(1);
			}
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
						if(!explode && posy > BHEIGHT/2)
							posy--;
						debug_printf("up\r\n");
						break;
					case 'B':
						if(!explode && posy < YSIZE-1 - (BHEIGHT/2))
							posy++;
						debug_printf("down\r\n");
						break;
					case 'D':
						if(!explode && posx < XSIZE-1-BWIDTH/2)
							posx++;
						debug_printf("right\r\n");
						break;
					case 'C':
						if(!explode && posx > BWIDTH/2)
							posx--;
						debug_printf("left\r\n");
						break;
					}
				}
				break;
			case 't':
			case ' ':
				add_shot();
				resetstate = 0;
				explodestate = 0;
				debug_printf("LT\r\n");
				break;
			case 'z':
				add_shot();
				resetstate = 0;
				explodestate = 0;
				debug_printf("RT\r\n");
				break;
			case 'g':
				add_shot();
				resetstate = 0;
				explodestate = 0;
				debug_printf("LB\r\n");
				break;
			case 'x':
				add_shot();
				resetstate = 0;
				explodestate = 0;
				debug_printf("RB\r\n");
				break;
			case '5':
				debug_printf("TT\r\n");
				if(resetstate == 0)
					resetstate++;
				else if(resetstate == 2) {
					col = 1;
					explode = MAXEXPLODE;
				} else
					resetstate = 0;
				if(explodestate == 0)
					explodestate = 1;
				else
					explodestate = 0;
				break;
			case '6':
				debug_printf("BB\r\n");
				if(resetstate == 1)
					resetstate++;
				else
					resetstate = 0;
				if(explodestate == 1)
					explodestate = 2;
				else
					explodestate = 0;
				break;
			case '7':
				debug_printf("LL\r\n");
				resetstate = 0;
				if(explodestate == 2)
					explodestate = 3;
				else
					explodestate = 0;
				break;
			case '8':
				debug_printf("RR\r\n");
				resetstate = 0;
				if(explodestate == 3) {
					col = 1;
				} else
					explodestate = 0;
				break;
			case 'Q':
				quit = 1;
				break;
			}
		}
		if(FD_ISSET(pfd[0], &fds)) {
			char c;
			timer_start();
			xread(pfd[0], &c, 1);
			if(col) {
				if(explode >= MAXEXPLODE) {
					animate_clear(fd);
					reset_game();
					col = 0;
				} else {
					memset(frontstore, 0, sizeof(frontstore));
					delaytime = DELAYTIMEMIN;
					put_border(0);
					put_shots(0);
					put_explode(explode++);
				}
			} else {
				memset(frontstore, 0, sizeof(frontstore));
				put_border(1);
				col = put_boat();
				shoot_wave();
				put_shots(1);
				active_step();
			}
			scr_frontmap(fd);
		}
	}

	timer_stop();
	close(fd);
	return 0;
}
