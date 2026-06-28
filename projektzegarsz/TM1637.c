/*
 * TM1637.c
 *
 * Created: 20.06.2026 19:59:44
 *  Author: Komp
 */ 
#define F_CPU 16000000UL
#include "TM1637.h"
#include <avr/io.h>
#include <util/delay.h>

// Definicje pinów 
#define CLK_A PORTD2
#define DIO_A PORTD3
#define CLK_B PORTD4
#define DIO_B PORTD5

// Mapa cyfr od 0 do 9 
const uint8_t segments[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

void TM1637_init(void) {
	// Ustawienie pinów jako wyjœcia i stan wysoki 
	DDRD |= (1<<CLK_A) | (1<<DIO_A) | (1<<CLK_B) | (1<<DIO_B);
	PORTD |= (1<<CLK_A) | (1<<DIO_A) | (1<<CLK_B) | (1<<DIO_B);
}

static void bit_delay(void) { _delay_us(5); }

static void start(uint8_t disp) {
	if(disp == 1) { PORTD &= ~(1<<DIO_A); bit_delay(); PORTD &= ~(1<<CLK_A); }
	else          { PORTD &= ~(1<<DIO_B); bit_delay(); PORTD &= ~(1<<CLK_B); }
	bit_delay();
}

static void stop(uint8_t disp) {
	if(disp == 1) { PORTD &= ~(1<<CLK_A); bit_delay(); PORTD &= ~(1<<DIO_A); bit_delay(); PORTD |= (1<<CLK_A); bit_delay(); PORTD |= (1<<DIO_A); }
	else          { PORTD &= ~(1<<CLK_B); bit_delay(); PORTD &= ~(1<<DIO_B); bit_delay(); PORTD |= (1<<CLK_B); bit_delay(); PORTD |= (1<<DIO_B); }
	bit_delay();
}

static void write_byte(uint8_t disp, uint8_t data) {
	for(uint8_t i=0; i<8; i++) {
		PORTD &= ~((disp==1)?(1<<CLK_A):(1<<CLK_B));
		bit_delay();
		if(data & 1) PORTD |= ((disp==1)?(1<<DIO_A):(1<<DIO_B));
		else         PORTD &= ~((disp==1)?(1<<DIO_A):(1<<DIO_B));
		bit_delay();
		PORTD |= ((disp==1)?(1<<CLK_A):(1<<CLK_B));
		bit_delay();
		data >>= 1;
	}
	PORTD &= ~((disp==1)?(1<<CLK_A):(1<<CLK_B));
	bit_delay();
	PORTD |= ((disp==1)?(1<<CLK_A):(1<<CLK_B)); // takt zegara
	bit_delay();
	PORTD &= ~((disp==1)?(1<<CLK_A):(1<<CLK_B));
	bit_delay();
}

void TM1637_show_time(uint8_t disp, uint8_t min, uint8_t sec, uint8_t colon) {
	uint8_t d0 = min / 10;
	uint8_t d1 = min % 10;
	uint8_t d2 = sec / 10;
	uint8_t d3 = sec % 10;

	uint8_t seg1 = segments[d1];
	if(colon) seg1 |= 0x80; // Zapalamy dwukropek (najstarszy bit)

	start(disp); write_byte(disp, 0x40); stop(disp); 
	start(disp); write_byte(disp, 0xC0);             // Ustaw adres na 0
	write_byte(disp, segments[d0]);                  // Cyfra 1 (dziesi¹tki minut)
	write_byte(disp, seg1);                          // Cyfra 2 (jednostki minut + ew. dwukropek)
	write_byte(disp, segments[d2]);                  // Cyfra 3 (dziesi¹tki sekund)
	write_byte(disp, segments[d3]);                  // Cyfra 4 (jednostki sekund)
	stop(disp);
	
	start(disp); write_byte(disp, 0x8F); stop(disp); // Ekran w³¹czony
}	