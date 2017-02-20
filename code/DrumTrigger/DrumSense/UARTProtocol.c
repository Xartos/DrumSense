/*
 * UARTProtocol.c
 *
 * Created: 2017-02-16 08:38:16
 *  Author: Fredde
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#define MYADDR 0x01

#include "UARTProtocol.h"

/*#ifndef MYADDR
#define MYADDR 0x01
#endif*/

// TODO: Set TX to output only when in active mode
// UART receive Interrupt
ISR(USART_RX_vect) {
	uint8_t byte = UDR0;
	
	if(UART.status != UART_STATUS_PART){
		// First byte received

		// Parse byte into fields
		UART.addr = byte >> 4;
		UART.command = byte & 0x0F;

		// Save last status and flag that first data is received
		UART.lastStatus = UART.status;
		UART.status = UART_STATUS_PART;

		//TODO: start timer and reset on interrupt
	} else {
		// Second byte received

		// Save data byte and restore status
		UART.data = byte;
		UART.status = UART.lastStatus;

		if (UART.addr == BROADADDR && UART.command == UART_COMMAND_STOP){
			// Master is broadcasting a stop
			UART.status = UART_STATUS_LISTENING;

		} else if(UART.status == UART_STATUS_LISTENING && UART.addr == MYADDR && UART.command == UART_COMMAND_START){
			// Master polled this device
			UART.status = UART_STATUS_ACTIVE;

			uartTransmittByte(UART_RESPONSE_ACK);
		} else if (UART.status == UART_STATUS_ACTIVE && UART.addr == MYADDR) {
			//  Master sent data to me, call user defined function
			uint8_t res = UART.RecieveCallback(UART.command, UART.data);

			if(res){
				uartTransmittByte(UART_RESPONSE_NACK);
				return;
			}
			
			uartTransmittByte(UART_RESPONSE_ACK);

		} else if (UART.status == UART_STATUS_LISTENING && UART.addr != MYADDR) {
			// Master polled another device, go to idle
			UART.status = UART_STATUS_IDLE;
		}
	}
}

void initUSART(){
	/*Set baud rate (19.2 Kbps) */
	UBRR0H = 0x00;
	UBRR0L = 0x19;
	 
	/* Enable dual speed (38.4 Kbps) */
	UCSR0A = (1 << U2X0);

	/*Enable receiver and transmitter */
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
	 
	/* Set frame format: 8data, 2stop bit */
	UCSR0C = (1 << USBS0) | (3 << UCSZ00);

	UART.status = UART_STATUS_IDLE;
}

void uartTransmittByte(uint8_t byte){
	//wait while previous byte is completed
	while(!(UCSR0A & (1 << UDRE0))){};
	// Transmit data
	UDR0 = byte;
}
