/*
 * uart.h
 *
 *  Created on: 10 сент. 2019 г.
 *      Author: islam
 */

#ifndef UART_H_
#define UART_H_

//#define F_CPU 8000000
#define BAUD 9600UL
#define RX_BUFFER_SIZE 32
#include "../main.h"
// UART
void UART_init(const uint16_t& speed);

// Передача одного байта
void UART_send_byte(uint8_t byte);

// Передача строки
void UART_send_string(const char* str);

// Передача строки с переводом каретки на новую строку
void UART_send_string_ln(const char* str);

unsigned char UART_Receive( void );
unsigned char UART_Receive_string( unsigned char *x, unsigned char size );

void USART_Init( unsigned int baud );
void USART_Transmit( unsigned char data );
void USART_send_string(const char* str);
void USART_send_string_ln(const char* str);
unsigned char USART_Receive( void );
void USART_Flush( void );

#endif /* UART_H_ */

