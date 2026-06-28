/*
 * TM1637.h
 *
 * Created: 20.06.2026 19:59:17
 *  Author: Komp
 */ 


#ifndef TM1637_H_
#define TM1637_H_
#include <stdint.h>

// Wybór wyœwietlacza
#define DISPLAY_A 1  // Lewy ekran (Gracz A)
#define DISPLAY_B 2  // Prawy ekran (Gracz B)

// G³ówne funkcje
void TM1637_init(void);
void TM1637_show_time(uint8_t disp, uint8_t min, uint8_t sec, uint8_t colon);

#endif /* TM1637_H_ */