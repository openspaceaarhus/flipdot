#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define BAUD_RATE 38400

static volatile unsigned char uart_buf[2];
static volatile unsigned char uart_i;

ISR(USART_RX_vect)
{
	uart_buf[uart_i] = UDR0;
	uart_i++;
}

static inline void uart_send(unsigned char dat)
{
	while(!(UCSR0A & ( 1 << UDRE0)));
	UDR0 = dat;
}

static inline unsigned char uart_receive(void)
{
	while (!(UCSR0A & (1<<RXC0)));
	return UDR0;
}

/* 

Connections:

IDC:

1, 2: Gnd
3: 5v
4, 12, 23, 24, 25: NC
26: 20v

Bertho Design:

18 = AD5/PC5 = B3.23, B4.23, B5.23, B6.23 ; Black/white select
13 = AD4/PC4 = B3.19, B4.19, B5.19, B6.19 ; Address
14 = AD3/PC3 = B3.18, B4.18, B5.18, B6.18 ; Address
15 = AD2/PC2 = B3.20, B4.20, B5.20, B6.20 ; Address
16 = AD1/PC1 = B3.21, B4.21, B5.21, B6.21 ; Address
17 = AD0/PC0 = B3.22, B4.22, B5.22, B6.22 ; Address

5 = Digital 8/PB0 = B1.19, B2.19 ; Address
6 = Digital 9/PB1 = B1.20, B2.20 ; Address
7 = Digital 10/PB2 = B1.18, B2.18 ; Address
8 = Digital 11/PB3 = B1.21, B2.21 ; Address
9 = Digital 12/PB4 = B1.22, B2.22 ; Address

10 = Digital 2/PD2 = B2.24 ; Board select
22 = Digital 3/PD3 = B6.24 ; Board select
11 = Digital 4/PD4 = B1.24 ; Board select
21 = Digital 5/PD5 = B5.24 ; Board select
19 = Digital 6/PD6 = B3.24 ; Board select
20 = Digital 7/PD7 = B4.24 ; Board select

*/

static const unsigned char x_index[] = {1, 4, 5, 2, 3,
					6, 7, 9, 12, 13,
					10, 11, 14, 15, 17,
					20, 21, 18, 19, 22,
					0,0,0,0,0};


static const unsigned char y_index[] = {
	0x10,0x08,0x18,0x04,0x14,0x0c,0x1c,0x12,0x0a,0x1a,0x06,0x16,0x0e,0x1e,0x11,0x09,
	0x19,0x05,0x15,0x0d,0x1d,0x13,0x0b,0x1b,0x07,0x17,0x0f,0x1f,0x10,0x08,0x18,0x04,
	0x14,0x0c,0x1c,0x12,0x0a,0x1a,0x06,0x16,0x0e,0x1e,0x11,0x09,0x19,0x05,0x15,0x0d,
	0x1d,0x13,0x0b,0x1b,0x07,0x17,0x0f,0x1f,0x10,0x08,0x18,0x04,0x14,0x0c,0x1c,0x12,
	0x0a,0x1a,0x06,0x16,0x0e,0x1e,0x11,0x09,0x19,0x05,0x15,0x0d,0x1d,0x13,0x0b,0x1b,
	0x07,0x17,0x0f,0x1f,0x10,0x08,0x18,0x04,0x14,0x0c,0x1c,0x12,0x0a,0x1a,0x06,0x16,
	0x0e,0x1e,0x11,0x09,0x19,0x05,0x15,0x0d,0x1d,0x13,0x0b,0x1b,0x07,0x17,0x0f,0x1f,
	0x10,0x08,0x18,0x04,0x14,0x0c,0x1c,0x12,
};

static const unsigned char y_board[] = {
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
	0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

static void
pixel (unsigned char x, unsigned char y, unsigned char set)
{
	PORTB = x_index[x & 0x1f];

	if (set) {
		/* Select Y Board and B1.24 */
		PORTC = y_index[y];
		PORTD = y_board[y] | 0x10;
	} else {
		/* Select Y Board, BX.23 and B2.24 */
		PORTC = y_index[y] | 0x20;
		PORTD = y_board[y] | 0x4;
	}

	_delay_us(320);

	PORTD = 0;
}

int main (void)
{
	uart_i = 0;
	
	/* Enable all outputs we need */
	DDRD = 0xfe;
	DDRB = 0x1f;
	DDRC = 0x3f;
	
	/* Set UART to BAUD_RATE, 8N1 */
	UBRR0H = ((F_CPU / 16 + BAUD_RATE / 2) / BAUD_RATE - 1) >> 8;
	UBRR0L = ((F_CPU / 16 + BAUD_RATE / 2) / BAUD_RATE - 1); 
	/* 8 data bits */
  	UCSR0C = (3<<UCSZ00);

	/* Rx/Tx Enable */
 	UCSR0B = (1<<TXEN0) | (1<<RXEN0) | (1 << RXCIE0);

	sei();

	while (1) {
		while(uart_i < 2);
		// One could argue that we need to disable interrupts here to avoid
		// race conditions. However bytes arrive slowly enough for this to work
		// fine
		uart_i = 0;
		pixel(uart_buf[0], uart_buf[1], uart_buf[0] & 0x80);
	}

	return 1;
}
