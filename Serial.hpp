/*
 * Serial.hpp
 *
 *  Created on: Jul 6, 2020
 *      Author: tako
 */

#ifndef SRC_SERIAL_HPP_
#define SRC_SERIAL_HPP_

#include "inttypes.h"
#include "usart.h"
#include "dma.h"
extern "C"{
#include <stdarg.h>
}
#include <array>
#include <map>

#define	_CR_CRLF		1	/* 1: Convert \n ==> \r\n in the output char */
#define	_USE_LONGLONG	0	/* 1: Enable long long integer in type "ll". */
#define	_LONGLONG_t		long long	/* Platform dependent long long integer type */

class Serial{
	public:
		Serial() = delete;
		Serial(UART_HandleTypeDef _huart,
				uint32_t _baudrate = 115200,
				uint32_t _Wordlength = UART_WORDLENGTH_8B,
				uint32_t _StopBits = UART_STOPBITS_1,
				uint32_t _Parity = UART_PARITY_NONE
		 );

		uint8_t available();
		uint8_t UART_getC();
		void UART_putC(const uint8_t buf);
		void xprintf (const char* fmt, ...);

	protected:
		static constexpr uint8_t DATANUM = 100;
		UART_HandleTypeDef huart;
		const uint32_t baudrate;
		const uint32_t Wordlength;
		const uint32_t StopBits;
		const uint32_t Parity;
		uint8_t serialData[DATANUM] = {};
		int16_t indexRead = 0;
		uint8_t cnt = 0;
		void init();

		void xputc (char c);
		void xputs (const char* str);

		size_t DW_CHAR	= sizeof(char);
		size_t DW_SHORT	= sizeof(short);
		size_t DW_LONG	= sizeof(long);

		void xvprintf (
			const char*	fmt,	/* Pointer to the format string */
			va_list arp			/* Pointer to arguments */
		);

};

Serial::Serial(UART_HandleTypeDef _huart,uint32_t _baudrate,uint32_t _Wordlength,uint32_t _StopBits,uint32_t _Parity):
		huart(_huart),baudrate(_baudrate),Wordlength(_Wordlength),StopBits(_StopBits),Parity(_Parity){
	huart.Init.BaudRate = _baudrate;
	huart.Init.WordLength = _Wordlength;
	huart.Init.StopBits = _StopBits;
	huart.Init.Parity = _Parity;

	HAL_UART_Init(&huart);
	HAL_UART_Receive_DMA(&huart,serialData,DATANUM);
}

uint8_t Serial::available(){
	uint8_t index = 0;
	uint8_t remainData = 0;

	index = DATANUM - huart.hdmarx->Instance->NDTR;
	remainData = index - indexRead;

	return remainData;
}

uint8_t Serial::UART_getC(){
	uint8_t readData = 0;
	readData = serialData[indexRead];
	indexRead++;

	if(indexRead >= DATANUM){
		indexRead = 0;
	}
	return readData;
}

void Serial::UART_putC(const uint8_t buf){
	HAL_UART_Transmit(&huart, (uint8_t *)&buf, sizeof(buf), 0xFF);
}


/*----------------------------------------------*/
/* Put a character                              */
/*----------------------------------------------*/

void Serial::xputc (char c)
{
	if (_CR_CRLF && c == '\n') xputc('\r');		/* CR -> CRLF */

	UART_putC((unsigned char)c);
}

void Serial::xputs (					/* Put a string to the default device */
	const char* str				/* Pointer to the string */
)
{
	while (*str) {
		xputc(*str++);
	}
}


void Serial::xvprintf (
	const char*	fmt,	/* Pointer to the format string */
	va_list arp			/* Pointer to arguments */
)
{
	unsigned int r, i, j, w, f;
	char s[24], c, d, *p;
#if _USE_LONGLONG
	_LONGLONG_t v;
	unsigned _LONGLONG_t vs;
#else
	long v;
	unsigned long vs;
#endif
	for (;;) {
		c = *fmt++;					/* Get a format character */
		if (!c) break;				/* End of format? */
		if (c != '%') {				/* Pass it through if not a % sequense */
			xputc(c); continue;
		}
		f = 0;						/* Clear flags */
		c = *fmt++;					/* Get first char of the sequense */
		if (c == '0') {				/* Flag: left '0' padded */
			f = 1; c = *fmt++;
		} else {
			if (c == '-') {			/* Flag: left justified */
				f = 2; c = *fmt++;
			}
		}
		for (w = 0; c >= '0' && c <= '9'; c = *fmt++) {	/* Minimum width */
			w = w * 10 + c - '0';
		}
		if (c == 'l' || c == 'L') {	/* Prefix: Size is long */
			f |= 4; c = *fmt++;
#if _USE_LONGLONG
			if (c == 'l' || c == 'L') {	/* Prefix: Size is long long */
				f |= 8; c = *fmt++;
			}
#endif
		}
		if (!c) break;				/* End of format? */
		d = c;
		if (d >= 'a') d -= 0x20;
		switch (d) {				/* Type is... */
		case 'S' :					/* String */
			p = va_arg(arp, char*);
			for (j = 0; p[j]; j++) ;
			while (!(f & 2) && j++ < w) xputc(' ');
			xputs(p);
			while (j++ < w) xputc(' ');
			continue;
		case 'C' :					/* Character */
			xputc((char)va_arg(arp, int)); continue;
		case 'B' :					/* Binary */
			r = 2; break;
		case 'O' :					/* Octal */
			r = 8; break;
		case 'D' :					/* Signed decimal */
		case 'U' :					/* Unsigned decimal */
			r = 10; break;
		case 'X' :					/* Hexdecimal */
			r = 16; break;
		default:					/* Unknown type (passthrough) */
			xputc(c); continue;
		}

		/* Get an argument and put it in numeral */
#if _USE_LONGLONG
		if (f & 8) {	/* long long argument? */
			v = va_arg(arp, _LONGLONG_t);
		} else {
			if (f & 4) {	/* long argument? */
				v = (d == 'D') ? (long)va_arg(arp, long) : (long)va_arg(arp, unsigned long);
			} else {		/* int/short/char argument */
				v = (d == 'D') ? (long)va_arg(arp, int) : (long)va_arg(arp, unsigned int);
			}
		}
#else
		if (f & 4) {	/* long argument? */
			v = va_arg(arp, long);
		} else {		/* int/short/char argument */
			v = (d == 'D') ? (long)va_arg(arp, int) : (long)va_arg(arp, unsigned int);
		}
#endif
		if (d == 'D' && v < 0) {	/* Negative value? */
			v = 0 - v; f |= 16;
		}
		i = 0; vs = v;
		do {
			d = (char)(vs % r); vs /= r;
			if (d > 9) d += (c == 'x') ? 0x27 : 0x07;
			s[i++] = d + '0';
		} while (vs != 0 && i < sizeof s);
		if (f & 16) s[i++] = '-';
		j = i; d = (f & 1) ? '0' : ' ';
		while (!(f & 2) && j++ < w) xputc(d);
		do xputc(s[--i]); while (i != 0);
		while (j++ < w) xputc(' ');
	}
}


void Serial::xprintf (			/* Put a formatted string to the default device */
	const char*	fmt,	/* Pointer to the format string */
	...					/* Optional arguments */
)
{
	va_list arp;
	va_start(arp, fmt);
	xvprintf(fmt, arp);
	va_end(arp);
}

#endif
