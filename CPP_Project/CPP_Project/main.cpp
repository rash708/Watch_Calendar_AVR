/*
 * GccApplication1.c
 *
 * Created: 01.10.2019 15:47:48
 * Author : Rashid
 */

#define F_CPU 8000000UL						  // Рабочая частота контроллера
#define BAUD 115200UL						  // Скорость обмена данными
#define UBRRL_value (F_CPU / (BAUD * 16)) - 1 //Согластно заданной скорости подсчитываем значение для регистра UBRR

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "UART/uart.h"

unsigned char second, minute, hour, date, month, disp_flag;
unsigned short year;
char rData, msg[50];
int count_char;
unsigned char set_date, set_time = 0;

int max_pos = 6;
unsigned char numberSegments[10] = {
	0x3f, //0
	0x06, //1
	0x5b, //2
	0x4f, //3
	0x66, //4
	0x6d, //5
	0x7d, //6
	0x07, //7
	0x7f, //8
	0x6f  //9
};

/*
	TCNT0 = 255(max_value_8bit) - (CPU_FREQ * Req.Time_Delay) / prescaler
	TCNT0 = 255 - (1000000(hz) * 100us)/1(no prescaling) // 100us = 100*10^-6sec
		  = 255 - 100  = 155 + 1 (extra clock)
*/

void timer0_delay()
{
	TCNT0 = 156; // 100us
	TCCR0 = (1 << CS00);
	while ((TIFR & 0x01) == 0x00)
		;		 //check flag if not set then wait
	TIFR = 0x01; //clear flag
}

static char not_leap(unsigned short year) //проверка вискосного года
{
	if (!(year % 100))
	{
		return (char)(year % 400);
	}
	else
	{
		return (char)(year % 4);
	}
}

//-----------------------------------------------------------------------------------------
//Блок прерываний
ISR(USART_RX_vect)
{
	rData = UDR;
	char temp[20] = {'\0'};
	int t_h, t_min, t_sec, t_date, t_month, t_year;
	//unsigned char flag;

	if (rData == '\r')
	{

		msg[count_char] = '\0';

		if (set_date == 1) //Если мы устанавливаем дату
		{
			sscanf(msg, "%d-%d-%d", &t_date, &t_month, &t_year);

			if ((t_date < 1) || (t_date > 31) || (t_month < 1) || (t_month > 12) || (t_year < 0))
			{
				UART_send_string_ln("Wrong value");
			}
			else if (t_date == 31)
			{
				if ((t_month == 2) || (t_month == 4) || (t_month == 6) || (t_month == 9) || (t_month == 11))
				{
					UART_send_string_ln("Wrong value of date for month");
				}
			}
			else if (t_date == 30)
			{
				if (t_month == 2)
				{
					UART_send_string_ln("Wrong value of date for month");
				}
			}
			else if (t_date == 29)
			{
				if ((t_month == 2) && (not_leap(t_year)))
				{
					UART_send_string_ln("Wrong value of date for month");
				}
				else
				{
					date = t_date;
					month = t_month;
					year = t_year;
					set_date = 0;
				}
			}
			else
			{
				date = t_date;
				month = t_month;
				year = t_year;
				set_date = 0;
			}

			//UART_send_string_ln((const char*)t_h);
		}
		else if (set_time == 1) //Если мы устанавливаем время
		{
			sscanf(msg, "%d-%d-%d", &t_h, &t_min, &t_sec);

			if ((t_h < 24) && (t_min < 60) && (t_sec < 60))
			{
				hour = t_h;
				minute = t_min;
				second = t_sec;
				set_time = 0;
			}
			else
			{
				UART_send_string_ln("Wrong value");
				UART_send_string("Print time (format 23-59-59): ");
			}

			//UART_send_string_ln((const char*)t_h);
		}
		else //Все остальные события
		{
			if (strcmp(msg, "help") == 0)
			{
				UART_send_string_ln("Command list:");
				UART_send_string_ln("print-date");
				UART_send_string_ln("print-time");
				UART_send_string_ln("set-date");
				UART_send_string_ln("set-time");
			}
			else if (strcmp(msg, "print-date") == 0)
			{
				UART_send_string_ln("date:");
				sprintf(temp, "%d-%d-%d", (int)date, (int)month, (int)year);
				UART_send_string_ln(temp);
			}
			else if (strcmp(msg, "print-time") == 0)
			{
				UART_send_string("time: ");
				sprintf(temp, "%d-%d-%d", (int)hour, (int)minute, (int)second);
				UART_send_string_ln(temp);
			}
			else if (strcmp(msg, "set-date") == 0)
			{
				UART_send_string("Print date (format 10-12-2019): ");
				set_date = 1;
			}
			else if (strcmp(msg, "set-time") == 0)
			{
				UART_send_string("Print time (format 23-59-59): ");
				set_time = 1;
			}
			else
			{
				UART_send_string_ln("Print 'help' for more information");
			}
		}

		count_char = 0;
	}
	else
	{
		msg[count_char] = rData;
		count_char++;
	}
}

// ISR(INT0_vect) //Внешнее прерывание по INT0 = PIND2 Меняет формат дату/время
// {
// 	disp_flag = !(disp_flag);
// }
//
// ISR(INT1_vect) //Внешнее прерывание по INT1 = PIND3 Меняет формат дату/время
// {
// 	disp_flag = !(disp_flag);
// }

ISR(TIMER1_COMPA_vect) //внешнее прерывание по таймеру t1 (каждую секунду) и обработка времени
{
	if (++second == 60)
	{
		second = 0;
		if (++minute == 60)
		{
			minute = 0;
			if (++hour == 24)
			{
				hour = 0;
				if (++date == 32)
				{
					month++;
					date = 1;
				}
				else if (date == 31)
				{
					if ((month == 4) || (month == 6) || (month == 9) || (month == 11))
					{
						month++;
						date = 1;
					}
				}
				else if (date == 30)
				{
					if (month == 2)
					{
						month++;
						date = 1;
					}
				}
				else if (date == 29)
				{
					if ((month == 2) && (not_leap(year)))
					{
						month++;
						date = 1;
					}
				}
				if (month == 13)
				{
					month = 1;
					year++;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------------------
//Блок настройки
void timer1_ini(void) // Настройка Timer1
{
	TCCR1B |= (1 << WGM12); // устанавливаем режим СТС (сброс по совпадению)
	TIMSK |= (1 << OCIE1A); //устанавливаем бит разрешения прерывания 1ого счетчика по совпадению с OCR1A(H и L)
	OCR1AH = 0b00010000;	//записываем в регистр число для сравнения
	OCR1AL = 0b00000010;
	TCCR1B |= (1 << CS12); //установим делитель 256.
}

/*void int_ini() //функция инициализации прерываний INT0, INT1
{
	MCUCR = (1<<ISC01) | (0<<ISC00); //По нисходящему для INT0
	MCUCR = (1<<ISC11) | (0<<ISC10); //По нисходящему для INT1
	GICR = (1 << INT1) | (1 << INT0);
}*/

void port_ini() //Настройка портов ввода/вывода
{
	DDRA = 0xff;  // PORTA as OUTPUT
	PORTA = 0x00; // All pins of PORTA LOW

	DDRC = 0xff;  // PORTC as OUTPUT
	PORTC = 0x00; // All pins of PORTC LOW

	DDRD = 0x00;  // PORTD as INPUT
	PORTD = 0xff; // All pins of PORTD HIGH
}

void var_ini() //Инициализация переменных
{
	second = 0;
	minute = 0;
	hour = 0;
	date = 26;
	month = 12;
	year = 2055;
	disp_flag = 0;
}
//-----------------------------------------------------------------------------------------

int main(void)
{
	timer1_ini();
	//int_ini();
	port_ini();
	var_ini();
	UART_init(8);

	sei(); //Глобальные прерывания


	UART_send_string_ln("Print 'help' for more information");

	while (1)
	{

		if (!(PIND & (1 << PIND2))) //Смена даты/время
		{
			if (disp_flag == 0)
				disp_flag = 1;
			else
				disp_flag = 0;
			while (!(PIND & (1 << PIND2)))
				;
		}

		//disp_flag для переключения отображения дата/время == 1/0
		if (disp_flag == 1)
		{

			if (!(PIND & (1 << PIND5)))
			{
				year++;
				while (!(PIND & (1 << PIND5)))
					;
			}

			if (!(PIND & (1 << PIND4)))
			{
				if (++month == 13)
					month = 1;
				date = 1;
				while (!(PIND & (1 << PIND4)))
					;
			}

			if (!(PIND & (1 << PIND3)))
			{
				if (++date == 32)
				{
					date = 1;
				}
				else if (date == 31)
				{
					if ((month == 4) || (month == 6) || (month == 9) || (month == 11))
					{
						date = 1;
					}
				}
				else if (date == 30)
				{
					if (month == 2)
					{
						date = 1;
					}
				}
				else if (date == 29)
				{
					if ((month == 2) && (not_leap(year)))
					{
						date = 1;
					}
				}
				while (!(PIND & (1 << PIND3)))
					;
			}

			PORTC = ~(1 << 5);
			PORTA = numberSegments[((year - 2000) % 10)]; //sec1
			_delay_ms(1);

			PORTC = ~(1 << 4);
			PORTA = numberSegments[((year - 2000) / 10)]; //sec2
			_delay_ms(1);

			PORTC = ~(1 << 3);
			PORTA = numberSegments[(month % 10)]; //min1
			_delay_ms(1);

			PORTC = ~(1 << 2);
			PORTA = numberSegments[(month / 10)]; //min2
			_delay_ms(1);

			PORTC = ~(1 << 1);
			PORTA = numberSegments[(date % 10)]; //hour1
			_delay_ms(1);

			PORTC = ~(1 << 0);
			PORTA = numberSegments[(date / 10)]; //hour2
			_delay_ms(1);
		}

		if (disp_flag == 0)
		{

			if (!(PIND & (1 << PIND5)))
			{
				if (++second == 60)
					second = 0;
				while (!(PIND & (1 << PIND5)))
					;
			}

			if (!(PIND & (1 << PIND4)))
			{
				if (++minute == 60)
					minute = 0;
				while (!(PIND & (1 << PIND4)))
					;
			}

			if (!(PIND & (1 << PIND3)))
			{
				if (++hour == 24)
					hour = 0;
				while (!(PIND & (1 << PIND3)))
					;
			}

			PORTC = ~(1 << 5);
			PORTA = numberSegments[(second % 10)]; //sec1
			_delay_ms(1);

			PORTC = ~(1 << 4);
			PORTA = numberSegments[(second / 10)]; //sec2
			_delay_ms(1);

			PORTC = ~(1 << 3);
			PORTA = numberSegments[(minute % 10)]; //min1
			_delay_ms(1);

			PORTC = ~(1 << 2);
			PORTA = numberSegments[(minute / 10)]; //min2
			_delay_ms(1);

			PORTC = ~(1 << 1);
			PORTA = numberSegments[(hour % 10)]; //hour1
			_delay_ms(1);

			PORTC = ~(1 << 0);
			PORTA = numberSegments[(hour / 10)]; //hour2
			_delay_ms(1);
		}
	}
}
