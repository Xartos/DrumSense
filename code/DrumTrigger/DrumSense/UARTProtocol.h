/*
 * UARTProtocol.h
 *
 * Created: 2017-02-16 08:38:28
 *  Author: Fredde
 */ 

/*#ifndef MYADDR
#error "Define a unique address for this devices!"
#endif*/

#ifndef UARTPROTOCOL_H_
#define UARTPROTOCOL_H_

#define BROADADDR 0x00

#define UART_COMMAND_STOP 0
#define UART_COMMAND_START 1
#define UART_COMMAND_DATA 2
#define UART_COMMAND_LEDR 3
#define UART_COMMAND_LEDG 4
#define UART_COMMAND_LEDB 5
/*#define UART_COMMAND_STOP 0
#define UART_COMMAND_STOP 0*/

#define UART_RESPONSE_ACK 0
#define UART_RESPONSE_NACK 1

#define UART_STATUS_LISTENING 0
#define UART_STATUS_IDLE 1
#define UART_STATUS_ACTIVE 2
#define UART_STATUS_PART 3
/*#define UART_STATUS_LISTEN 0
#define UART_STATUS_LISTEN 0
#define UART_STATUS_LISTEN 0
#define UART_STATUS_LISTEN 0*/

typedef struct UART_t {
	uint8_t status; // Master init with a stop command
	uint8_t lastStatus;
	uint8_t (*RecieveCallback)(uint8_t, uint8_t);
	uint8_t addr;
	uint8_t command;
	uint8_t data;
} UART_t;

UART_t UART;

void initUSART();
void uartTransmittByte(uint8_t byte);


#endif /* UARTPROTOCOL_H_ */