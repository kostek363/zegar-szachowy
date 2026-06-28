#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "TM1637.h"

// ==========================================
// ZMIENNE GLOBALNE I USTAWIENIA
// ==========================================
const uint8_t opcje_czasu[] = {1, 3, 10};
uint8_t wybrana_opcja = 1;

volatile int8_t min_A = 3, sec_A = 0;
volatile int8_t min_B = 3, sec_B = 0;

// Status gry: 0 = Ustawienia/Pauza, 1 = Tura Lewego, 2 = Tura Prawego
volatile uint8_t aktywny_gracz = 0;
volatile uint8_t flaga_konca_czasu = 0; // Informuje g≥Ûwnπ pÍtlÍ o przegranej
volatile uint8_t flaga_odswiezania = 1; // Flaga do optymalizacji wyúwietlacza

// ==========================================
// FUNKCJE DèWI KOWE
// ==========================================
void beep_krotki(void) {
	PORTB |= (1 << PORTB3);  // W≥πcz buzzer
	_delay_ms(50);           // Czekaj 50 ms
	PORTB &= ~(1 << PORTB3); // Wy≥πcz buzzer
}

void beep_dlugi(void) {
	PORTB |= (1 << PORTB3);
	_delay_ms(500);         // Czekaj sekunde
	PORTB &= ~(1 << PORTB3);
}

// ==========================================
// FUNKCJE POMOCNICZE
// ==========================================
void konfiguracja_sprzetu(void)
{
	// Przyciski jako wejúcia (PB0, PB1, PB2)
	DDRB &= ~((1 << PORTB0) | (1 << PORTB1) | (1 << PORTB2));
	PORTB |= (1 << PORTB0) | (1 << PORTB1) | (1 << PORTB2);
	
	// Buzzer (PB3) jako WYJåCIE
	DDRB |= (1 << PORTB3);
	PORTB &= ~(1 << PORTB3); // Domyúlnie wy≥πczony

	TM1637_init();
}

void konfiguracja_timera(void)
{
	TCCR1B |= (1 << WGM12);
	TCCR1B |= (1 << CS12) | (1 << CS10);
	OCR1A = 15624;
	TIMSK1 |= (1 << OCIE1A);
	sei();
}

// ==========================================
// PRZERWANIE 
// ==========================================
ISR(TIMER1_COMPA_vect)
{
	if (aktywny_gracz == 1) {
		if (sec_A == 0) {
			if (min_A > 0) { min_A--; sec_A = 59; }
			else { aktywny_gracz = 0; flaga_konca_czasu = 1; } // Koniec czasu!
			} else { sec_A--; }
			flaga_odswiezania = 1; // ZmieÒ flagÍ, bo up≥ynÍ≥a sekunda
		}
		else if (aktywny_gracz == 2) {
			if (sec_B == 0) {
				if (min_B > 0) { min_B--; sec_B = 59; }
				else { aktywny_gracz = 0; flaga_konca_czasu = 1; } // Koniec czasu!
				} else { sec_B--; }
				flaga_odswiezania = 1; // ZmieÒ flagÍ, bo up≥ynÍ≥a sekunda
			}
		}

		// ==========================================
		// G£”WNA P TLA PROGRAMU
		// ==========================================
		int main(void)
		{
			konfiguracja_sprzetu();
			konfiguracja_timera();

			while (1)
			{
				// --- 0. SPRAWDZENIE KO—CA CZASU ---
				if (flaga_konca_czasu == 1) {
					beep_dlugi();          // D≥ugi, alarmowy düwiÍk
					flaga_konca_czasu = 0; // Kasujemy flagÍ po odtworzeniu düwiÍku
					flaga_odswiezania = 1; // Odúwieø ekrany na koniec gry
				}

				// --- 1. PRZYCISK CZARNY (PB2) - USTAWIENIA / RESET ---
				if (!(PINB & (1 << PINB2)))
				{
					if (aktywny_gracz == 0) {
						wybrana_opcja++;
						if (wybrana_opcja > 2) wybrana_opcja = 0;
						beep_krotki(); // PikniÍcie przy zmianie czasu
						} else {
						aktywny_gracz = 0; // RESET z trwajπcej gry
						beep_dlugi();      // Inny düwiÍk dla resetu
					}
					
					min_A = opcje_czasu[wybrana_opcja]; sec_A = 0;
					min_B = opcje_czasu[wybrana_opcja]; sec_B = 0;
					flaga_odswiezania = 1; // Wymuú odúwieøenie ekranÛw
					_delay_ms(300);
				}

				// --- 2. PRZYCISKI GRACZY ---
				if ((min_A == 0 && sec_A == 0) || (min_B == 0 && sec_B == 0)) { //blokada gry po koÒcu czasu
				}
				else if (!(PINB & (1 << PINB0))) {
					if (aktywny_gracz != 2) {
						beep_krotki(); // Piknij, jeúli to faktyczna zmiana
						flaga_odswiezania = 1; // Wymuú odúwieøenie po zmianie gracza
					}
					aktywny_gracz = 2;
					_delay_ms(200);
				}
				else if (!(PINB & (1 << PINB1))) {
					if (aktywny_gracz != 1) {
						beep_krotki(); // Piknij, jeúli to faktyczna zmiana
						flaga_odswiezania = 1; // Wymuú odúwieøenie po zmianie gracza
					}
					aktywny_gracz = 1;
					_delay_ms(200);
				}
				
				// --- 3. AKTUALIZACJA WYåWIETLACZY ---
				// Kod wykona siÍ tylko wtedy, gdy flaga odúwieøania wynosi 1
				if (flaga_odswiezania == 1) {
					if (aktywny_gracz == 1) {
						TM1637_show_time(DISPLAY_A, min_A, sec_A, 1);
						TM1637_show_time(DISPLAY_B, min_B, sec_B, 0);
					}
					else if (aktywny_gracz == 2) {
						TM1637_show_time(DISPLAY_A, min_A, sec_A, 0);
						TM1637_show_time(DISPLAY_B, min_B, sec_B, 1);
					}
					else {
						TM1637_show_time(DISPLAY_A, min_A, sec_A, 1);
						TM1637_show_time(DISPLAY_B, min_B, sec_B, 1);
					}
					
					flaga_odswiezania = 0; // Wyzeruj flagÍ
				}
			}
		}