/*
 * main.h
 *
 *  Created on: 16 окт. 2019 г.
 *      Author: islam
 */

#ifndef SOURCE_MAIN_H_
#define SOURCE_MAIN_H_
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <stdio.h>
#include <string.h>


typedef unsigned char uint8_t;
typedef unsigned int uint16_t;


void init_avr();
void init_timer2();

#endif /* SOURCE_MAIN_H_ */
