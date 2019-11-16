/*
 * uart.cpp
 *
 *  Created on: 9 окт. 2019 г.
 *      Author: islam
 */

#include "uart.h"

// UART
void UART_init(const uint16_t& speed) {
	UBRRH = (uint8_t)(speed>>8);
	UBRRL = (uint8_t)(speed);
	//   Enable RXD | Enable TXD
	UCSRB = (1<<RXEN) | (1<<TXEN) | (1<<RXCIE); // Receiver / Transmitter / Receiver Interrupt
//	UCSRA = 0;
	UCSRA |= (1<<U2X);
	//       UCSRC   |        Even       |   8 bits in frame
	UCSRC = 1<<URSEL | 1<<UPM1 | 0<<UPM0 | 1<<UCSZ0 | 1<<UCSZ1;
}

//  Передача одного байта
void UART_send_byte(const uint8_t byte) {
  while(!( UCSRA & (1<<UDRE)));
  UDR = byte;
}


//  Передача строки
void UART_send_string(const char* str) {
	while (*str != 0) {
		UART_send_byte(*str++);
	}
}


//  Передача строки с переводом каретки на новую строку
void UART_send_string_ln(const char* str) {
  UART_send_string(str);
  UART_send_byte(0x0D); // carriage return
  UART_send_byte(0x0A); // new line
}

// Получение данных
unsigned char UART_Receive( void )
{
	/* Wait for data to be received */
	while ( !(UCSRA & (1<<RXC)) )
	;
	/* Get and return received data from buffer */
	return UDR;
}

unsigned char UART_Receive_string( unsigned char *x, unsigned char size )
{
	unsigned char i = 0;
	unsigned char c;
	
	if (size == 0) return 0;
	
	while (i < size - 1) {              // check space is available (including additional null char at end)
        c = UART_Receive();
        if (c == '!') break;           // break on ! character
        x[i] = c;                       // write into the supplied buffer
        i++;
    }
	x[i] = 0;                           // ensure string is null terminated

	return i + 1;                       // return number of characters written
}


//USART
void USART_Init( unsigned int baud )
{
	/* Set baud rate */
	UBRRH = (unsigned char)(baud>>8);
	UBRRL = (unsigned char)baud;
	/* Enable receiver and transmitter */
	UCSRA |= (1<<U2X);
	UCSRB = (1<<RXEN)|(1<<TXEN);
	/* Set frame format: 8data, 2stop bit */
	UCSRC = (1<<URSEL)|(1<<USBS)|(3<<UCSZ0);
}

void USART_Transmit( unsigned char data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSRA & (1<<UDRE)) )
	;
	/* Put data into buffer, sends the data */
	UDR = data;
}

//  Передача строки
void USART_send_string(const char* str) {
	while (*str != 0) {
		USART_Transmit(*str++);
	}
}


//  Передача строки с переводом каретки на новую строку
void USART_send_string_ln(const char* str) {
	USART_send_string(str);
	USART_Transmit(0x0D); // carriage return
	USART_Transmit(0x0A); // new line
}


unsigned char USART_Receive( void )
{
	/* Wait for data to be received */
	while ( !(UCSRA & (1<<RXC)) )
	;
	/* Get and return received data from buffer */
	return UDR;
}

void USART_Flush( void )
{
	unsigned char dummy;
	while ( UCSRA & (1<<RXC) ) dummy = UDR;
}

