/*
 * DrumSens.c
 *
 * Created: 2017-02-13 19:12:29
 * Author : Fredde
 */ 

#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h> //_delay_ms
#include <stdlib.h>		//rand

#include "UARTProtocol.h"

#define ADCTHRESHOLD (1024 / 2)
#define LEDFADE 8
#define LEDMAX 0xFF

// Glow that at least is present
#define LEDGLOWR 0x10
#define LEDGLOWG 0x10
#define LEDGLOWB 0x10

typedef struct LED_t {
	uint8_t	R;
	uint8_t	G;
	uint8_t	B;

	uint8_t	newColor;
} LED_t;

void initLEDs();
void initADCs();
uint16_t readADC();
uint8_t UARTData(uint8_t byte);
void getRandomStartColor(LED_t* leds);
void bumpLEDS(LED_t* leds);
void decreaseLED(LED_t* leds);
void writeLED(LED_t* leds);

int main(void){
	LED_t leds;

	leds.R = LEDGLOWR;
	leds.G = LEDGLOWG;
	leds.B = LEDGLOWB;
	leds.newColor = 1;

	UART.RecieveCallback = UARTData;

	// Initialize
	initLEDs();
	initADCs();
	initUSART();

	// Wait for ADC to make some readings
	_delay_ms(100);

	// Seed for random generator. Is this good enough? 
	srand(ADC);
    
    while (1) {
		// Read ADCs
		uint16_t currRead = readADC();

		// Update LEDStatus if threshold is reached
		if(currRead > ADCTHRESHOLD){
			if(leds.newColor){
				// New color is needed
				getRandomStartColor(&leds);
				leds.newColor = 0;
			} else {
				// Normalize values to LEDMAX
				bumpLEDS(&leds);
			}
		}

		// Write to LED
		writeLED(&leds);

		// Decrease LEDStatus every loop
		decreaseLED(&leds);

		// Sleep a while
		_delay_ms(10);
    }
}

/*
 * Initializes the PWM for the LED-strip
 */
void initLEDs(){
	// Set PB1, 2 and 3 to output
	DDRB |= (1 << DDB1) | (1 << DDB2) | (1 << DDB3);

	// Reset PWM registers
	OCR2A = 0;
	OCR1A = 0;
	OCR1B = 0;
	
	// Fast PWM, 8 bit, clock scale 1. For OC1A and OC1B
	TCCR1A |= (1 << COM1A1) | (1 << WGM10);
	TCCR1B |= (1 << CS10) | (1 << WGM12);

	// Fast PWM, 8 bit, clock scale 1. For OC2A 
	TCCR2A |= (1 << COM2A1) | (1 << WGM20) | (1 << WGM21);
	TCCR2B |= (1 << CS20);

	// Set PWM registers to default glow
	OCR2A = LEDGLOWR;
	OCR1A = LEDGLOWG;
	OCR1B = LEDGLOWB;
}

/*
 * Initializes the ADC for the mic reading
 */
void initADCs(){
	// Select ADC5
	ADMUX |= (1 << MUX2) | (1 << MUX0);

	// Enable and start ADC in free running mode. 
	ADCSRA |= (1 << ADEN) | (1 << ADSC);
}

/*
 * Returns the value of the 10-bit ADC register
 */
uint16_t readADC(){
	return ADC;
}

uint8_t UARTData(uint8_t byte){
	return 0;
}


/*
 * Randomizes a color with at least one of the colors set to LEDMAX
 */
void getRandomStartColor(LED_t* leds){
	uint16_t first = rand();
	uint16_t second = rand();
	uint16_t third = rand();

	
	if(first >= second){
		if (first >= third){
			// First is biggest
			leds->R = first / (RAND_MAX / LEDMAX + 1);
			leds->G = second / (LEDMAX / leds->R + 1);
			leds->B = third / (LEDMAX / leds->R + 1);
			return;
		}

		// Second is biggest
		leds->R = first / (LEDMAX / leds->G + 1);
		leds->G = second / (RAND_MAX / LEDMAX + 1);
		leds->B = third / (LEDMAX / leds->G + 1);
		return;
	} else if(second >= third){
		// Second is biggest
		leds->R = first / (LEDMAX / leds->G + 1);
		leds->G = second / (RAND_MAX / LEDMAX + 1);
		leds->B = third / (LEDMAX / leds->G + 1);
		return;
	}

	// Third is biggest
	leds->R = first / (LEDMAX / leds->B + 1);
	leds->G = second / (LEDMAX / leds->B + 1);
	leds->B = third / (RAND_MAX / LEDMAX + 1);
}

void bumpLEDS(LED_t* leds){
	uint8_t diff = 0;

	if(leds->R >= leds->G){
		if (leds->R >= leds->B){
			// R is largest
			diff = LEDMAX - leds->R;
		} else {
			// G is largest
			diff = LEDMAX - leds->G;
		}
	} else {
		if(leds->G >= leds->B){
			// G is largest
			diff = LEDMAX - leds->G;
		} else {
			// B is largest
			diff = LEDMAX - leds->B;
		}
	}

	leds->R += diff;
	leds->G += diff;
	leds->B += diff;
}

void decreaseLED(LED_t* leds){
	leds->R = (leds->R > LEDFADE) ? (leds->R - LEDFADE) : 0;
	leds->G = (leds->G > LEDFADE) ? (leds->G - LEDFADE) : 0;
	leds->B = (leds->B > LEDFADE) ? (leds->B - LEDFADE) : 0;
}

void writeLED(LED_t* leds){
	if((leds->R <= LEDGLOWR) && (leds->G <= LEDGLOWG) && (leds->B <= LEDGLOWB)){
		leds->newColor = 1;
	}

	OCR2A = (leds->R > LEDGLOWR) ? leds->R : LEDGLOWR;
	OCR1B = (leds->G > LEDGLOWG) ? leds->G : LEDGLOWG;
	OCR1A = (leds->B > LEDGLOWB) ? leds->B : LEDGLOWB;
}