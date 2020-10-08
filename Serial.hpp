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
		 ): huart(_huart),baudrate(_baudrate),Wordlength(_Wordlength),StopBits(_StopBits),Parity(_Parity){
			huart.Init.BaudRate = _baudrate;
			huart.Init.WordLength = _Wordlength;
			huart.Init.StopBits = _StopBits;
			huart.Init.Parity = _Parity;

			HAL_UART_Init(&huart);
			HAL_UART_Receive_DMA(&huart,serialData,DATANUM);
		};

		uint8_t available(){
			uint8_t index = DATANUM - huart.hdmarx->Instance->NDTR;;
			uint8_t remainData = index - indexRead;
			return remainData;
		};

		uint8_t read(){
			uint8_t readData = serialData[indexRead];
			indexRead++;
			if(indexRead >= DATANUM) indexRead = 0;
			return readData;
		};

		void write(const uint8_t buf){ HAL_UART_Transmit(&huart, (uint8_t *)&buf, sizeof(buf), 0xFF); };
		void write(uint8_t* buf,size_t size) { for(size_t i = 0;i < size;i++) write(buf[i]); };
		void write(std::vector<uint8_t> buf){ for(auto &i:buf) write(i); };

		void printf (const char* fmt, ...)
		{
			va_list arp;
			va_start(arp, fmt);
			xvprintf(fmt, arp);
			va_end(arp);
		};

		void print(const char fmt){ write(fmt); };
		void print(const int16_t fmt) { printf("%d",fmt); };
		void print(const std::string fmt) { printf(fmt.c_str()); };
		void print(const char* fmt){ printf(fmt); };
		void print(const float fmt,size_t precision = 2){ print(static_cast<double>(fmt),precision); };
		void print(const double fmt,size_t precision = 2){
			char temp[128] = {0};
			ftoa(static_cast<double>(fmt),temp,precision);
			printf("%s",temp);
		};

		void println(const char fmt){
			write(fmt);
			printf("\n");
		};
		void println(const int16_t fmt) { printf("%d\n",fmt); };
		void println(const std::string fmt) {
			printf(fmt.c_str());
			printf("\n");
		};
		void println(const char* fmt){
			printf(fmt);
			printf("\n");
		};
		void println(const float fmt,size_t precision = 2){
			println(static_cast<double>(fmt),precision);
		};
		void println(const double fmt,size_t precision = 2){
			char temp[128] = {0};
			ftoa(static_cast<double>(fmt),temp,precision);
			printf("%s\n",temp);
		};

	protected:
		static constexpr size_t MAX_PRECISION = 10;
		const double rounders[10 + 1] =
		{
			0.5,				// 0
			0.05,				// 1
			0.005,				// 2
			0.0005,				// 3
			0.00005,			// 4
			0.000005,			// 5
			0.0000005,			// 6
			0.00000005,			// 7
			0.000000005,		// 8
			0.0000000005,		// 9
			0.00000000005		// 10
		};
		static constexpr uint8_t DATANUM = 100;
		static constexpr size_t DW_CHAR	= sizeof(char);
		static constexpr size_t DW_SHORT	= sizeof(short);
		static constexpr size_t DW_LONG	= sizeof(long);

		UART_HandleTypeDef huart;
		const uint32_t baudrate;
		const uint32_t Wordlength;
		const uint32_t StopBits;
		const uint32_t Parity;
		uint8_t serialData[DATANUM] = {};
		int16_t indexRead = 0;
		uint8_t cnt = 0;
		void init();

		void xputc (char c){
			if (_CR_CRLF && c == '\n') xputc('\r');		/* CR -> CRLF */

			write((unsigned char)c);
		};
		void xputs (const char* str){
			while (*str) {
				xputc(*str++);
			}
		};

		void xvprintf (
			const char*	fmt,	/* Pointer to the format string */
			va_list arp			/* Pointer to arguments */
		){
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

		};

		char* ftoa(double f, char * buf,size_t precision)
		{
			char * ptr = buf;
			char * p = ptr;
			char * p1;
			char c;
			long intPart;

			// check precision bounds
			if (precision > MAX_PRECISION) precision = MAX_PRECISION;

			// sign stuff
			if (f < 0)
			{
				f = -f;
				*ptr++ = '-';
			}

			if (precision < 0)  // negative precision == automatic precision guess
			{
				if (f < 1.0) precision = 6;
				else if (f < 10.0) precision = 5;
				else if (f < 100.0) precision = 4;
				else if (f < 1000.0) precision = 3;
				else if (f < 10000.0) precision = 2;
				else if (f < 100000.0) precision = 1;
				else precision = 0;
			}

			// round value according the precision
			if (precision)
				f += rounders[precision];

			// integer part...
			intPart = f;
			f -= intPart;

			if (!intPart)
				*ptr++ = '0';
			else
			{
				// save start pointer
				p = ptr;

				// convert (reverse order)
				while (intPart)
				{
					*p++ = '0' + intPart % 10;
					intPart /= 10;
				}

				// save end pos
				p1 = p;

				// reverse result
				while (p > ptr)
				{
					c = *--p;
					*p = *ptr;
					*ptr++ = c;
				}

				// restore end pos
				ptr = p1;
			}

			// decimal part
			if (precision)
			{
				// place decimal point
				*ptr++ = '.';

				// convert
				while (precision--)
				{
					f *= 10.0;
					c = f;
					*ptr++ = '0' + c;
					f -= c;
				}
			}

			// terminating zero
			*ptr = 0;

			return buf;
		};

};

#endif
