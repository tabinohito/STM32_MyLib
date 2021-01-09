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
#include <string.h>
#include <math.h>

#define	XF_CRLF			1	/* 1: Convert \n ==> \r\n in the output char */
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

		UART_HandleTypeDef getHandle(){
			return huart;
		};

		uint8_t available(){
			uint8_t index = DATANUM - huart.hdmarx->Instance->CNDTR;
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

		void printf (const char* fmt, ...)
		{
			va_list arp;
			va_start(arp, fmt);
			xvfprintf(fmt, arp);
			va_end(arp);
		};

		void print(const char fmt){ write(fmt); };
		void print(const uint8_t fmt) { print(std::to_string(fmt)); };
		void print(const uint16_t fmt) { print(std::to_string(fmt)); };
		void print(const uint32_t fmt) { print(std::to_string(fmt)); };
		void print(const int8_t fmt) { print(std::to_string(fmt)); };
		void print(const int16_t fmt) { print(std::to_string(fmt)); };
		void print(const int32_t fmt) { print(std::to_string(fmt)); };
		void print(const std::string fmt) { printf(fmt.c_str()); };
		void print(const char* fmt){ printf(fmt); };
		void print(const float fmt){ printf("%f",fmt); };

		void println(){ printf("\n"); };
		void println(const char fmt){
			write(fmt);
			printf("\n");
		};
		void println(const uint8_t fmt) { print(std::to_string(fmt)); println();};
		void println(const uint16_t fmt) { print(std::to_string(fmt)); println();};
		void println(const uint32_t fmt) { print(std::to_string(fmt)); println();};
		void println(const int8_t fmt) { print(std::to_string(fmt)); println();};
		void println(const int16_t fmt) { print(std::to_string(fmt)); println();};
		void println(const int32_t fmt) { print(std::to_string(fmt)); println();};
		void println(const std::string fmt) {
			printf(fmt.c_str());
			printf("\n");
		};
		void println(const char* fmt){
			printf(fmt);
			printf("\n");
		};
		void println(const float fmt){
			printf("%f\n",fmt);
		};

	protected:
		static constexpr size_t MAX_PRECISION = 10;
		static constexpr size_t SZB_OUTPUT = 32;
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
			if (XF_CRLF && c == '\n') xputc('\r');		/* CR -> CRLF */

			write((unsigned char)c);
		};

		void xputc (
			uint8_t  chr				/* Character to be output */
		)
		{
			xfputc(chr);	/* Output it to the default output device */
		}


		void xfputc (			/* Put a character to the specified device */
			uint8_t chr				/* Character to be output */
		)
		{
			if (XF_CRLF && chr == '\n') xfputc('\r');	/* CR -> CRLF */
			write(chr);		/* Write a character to the output device */
		}



		/*----------------------------------------------*/
		/* Put a null-terminated string                 */
		/*----------------------------------------------*/


		void xvfprintf (
			const char*	fmt,	/* Pointer to the format string */
			va_list arp			/* Pointer to arguments */
		)
		{
			unsigned int r, i, j, w, f;
			int n, prec;
			char str[SZB_OUTPUT], c, d, *p, pad;
		#if XF_USE_LLI
			XF_LLI_t v;
			unsigned XF_LLI_t vs;
		#else
			long v;
			unsigned long vs;
		#endif

			for (;;) {
				c = *fmt++;					/* Get a format character */
				if (!c) break;				/* End of format? */
				if (c != '%') {				/* Pass it through if not a % sequense */
					xfputc(c); continue;
				}
				f = w = 0;			 		/* Clear parms */
				pad = ' '; prec = -1;
				c = *fmt++;					/* Get first char of the sequense */
				if (c == '0') {				/* Flag: left '0' padded */
					pad = '0'; c = *fmt++;
				} else {
					if (c == '-') {			/* Flag: left justified */
						f = 2; c = *fmt++;
					}
				}
				if (c == '*') {				/* Minimum width from an argument */
					n = va_arg(arp, int);
					if (n < 0) {			/* Flag: left justified */
						n = 0 - n; f = 2;
					}
					w = n; c = *fmt++;
				} else {
					while (c >= '0' && c <= '9') {	/* Minimum width */
						w = w * 10 + c - '0';
						c = *fmt++;
					}
				}
				if (c == '.') {				/* Precision */
					c = *fmt++;
					if (c == '*') {				/* Precision from an argument */
						prec = va_arg(arp, int);
						c = *fmt++;
					} else {
						prec = 0;
						while (c >= '0' && c <= '9') {
							prec = prec * 10 + c - '0';
							c = *fmt++;
						}
					}
				}
				if (c == 'l') {		/* Prefix: Size is long */
					f |= 4; c = *fmt++;
		#if XF_USE_LLI
					if (c == 'l') {	/* Prefix: Size is long long */
						f |= 8; c = *fmt++;
					}
		#endif
				}
				if (!c) break;				/* End of format? */
				switch (c) {				/* Type is... */
				case 'b':					/* Unsigned binary */
					r = 2; break;
				case 'o':					/* Unsigned octal */
					r = 8; break;
				case 'd':					/* Signed decimal */
				case 'u':					/* Unsigned decimal */
					r = 10; break;
				case 'x':					/* Hexdecimal (abc) */
				case 'X':					/* Hexdecimal (ABC) */
					r = 16; break;
				case 'c':					/* A character */
					xfputc((char)va_arg(arp, int)); continue;
				case 's':					/* String */
					p = va_arg(arp, char*);
					for (j = strlen(p); !(f & 2) && j < w; j++) xfputc(pad);	/* Left pads */
					while (*p && prec--) xfputc(*p++);/* String */
					while (j++ < w) xfputc(' ');		/* Right pads */
					continue;
				case 'f':					/* Float (decimal) */
				case 'e':					/* Float (e) */
				case 'E':					/* Float (E) */
					ftoa(p = str, va_arg(arp, double), prec, c);	/* Make fp string */
					for (j = strlen(p); !(f & 2) && j < w; j++) xfputc(pad);	/* Left pads */
					do xfputc(*p++); while (*p);	/* Value */
					while (j++ < w) xfputc(' ');	/* Right pads */
					continue;
				default:					/* Unknown type (passthrough) */
					xfputc(c); continue;
				}

				/* Get an argument and put it in numeral */
		#if XF_USE_LLI
				if (f & 8) {	/* long long argument? */
					v = va_arg(arp, XF_LLI_t);
				} else {
					if (f & 4) {	/* long argument? */
						v = (c == 'd') ? (XF_LLI_t)va_arg(arp, long) : (XF_LLI_t)va_arg(arp, unsigned long);
					} else {		/* int/short/char argument */
						v = (c == 'd') ? (XF_LLI_t)va_arg(arp, int) : (XF_LLI_t)va_arg(arp, unsigned int);
					}
				}
		#else
				if (f & 4) {	/* long argument? */
					v = va_arg(arp, long);
				} else {		/* int/short/char argument */
					v = (c == 'd') ? (long)va_arg(arp, int) : (long)va_arg(arp, unsigned int);
				}
		#endif
				if (c == 'd' && v < 0) {	/* Negative value? */
					v = 0 - v; f |= 1;
				}
				i = 0; vs = v;
				do {	/* Make value string */
					d = (char)(vs % r); vs /= r;
					if (d > 9) d += (c == 'x') ? 0x27 : 0x07;
					str[i++] = d + '0';
				} while (vs != 0 && i < sizeof str);
				if (f & 1) str[i++] = '-';				/* Sign */
				for (j = i; !(f & 2) && j < w; j++) xfputc(
						pad);	/* Left pads */
				do xfputc(str[--i]); while (i != 0);	/* Value */
				while (j++ < w) xfputc(' ');			/* Right pads */
			}
		}

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

		int ilog10 (double n)	/* Calculate log10(n) in integer output */
		{
			int rv = 0;

			while (n >= 10) {	/* Decimate digit in right shift */
				if (n >= 100000) {
					n /= 100000; rv += 5;
				} else {
					n /= 10; rv++;
				}
			}
			while (n < 1) {		/* Decimate digit in left shift */
				if (n < 0.00001) {
					n *= 100000; rv -= 5;
				} else {
					n *= 10; rv--;
				}
			}
			return rv;
		};


		double i10x (int n)	/* Calculate 10^n */
		{
			double rv = 1;

			while (n > 0) {		/* Left shift */
				if (n >= 5) {
					rv *= 100000; n -= 5;
				} else {
					rv *= 10; n--;
				}
			}
			while (n < 0) {		/* Right shift */
				if (n <= -5) {
					rv /= 100000; n += 5;
				} else {
					rv /= 10; n++;
				}
			}
			return rv;
		};


		void ftoa (
			char* buf,	/* Buffer to output the generated string */
			double val,	/* Real number to output */
			int prec,	/* Number of fractinal digits */
			char fmt	/* Notation */
		)
		{
			int d;
			int e = 0, m = 0;
			double w;
			const char *er = 0;


			if (isnan(val)) {			/* Not a number? */
				er = "NaN";
			} else {
				if (prec < 0) prec = 6;	/* Default precision (6 fractional digits) */
				if (val < 0) {			/* Nagative? */
					val = -val; *buf++ = '-';
				}
				if (isinf(val)) {		/* Infinite? */
					er = "INF";
				} else {
					if (fmt != 'f') {	/* E notation? */
						if (val != 0) {		/* Not true zero? */
							val += i10x(ilog10(val) - prec) / 2;	/* Round */
							e = ilog10(val);
							if (e > 99 || prec + 7 > SZB_OUTPUT) {
								er = "OV";
							} else {
								if (e < -99) e = -99;
								val /= i10x(e);	/* Normalize */
							}
						}
					} else {			/* Decimal notation */
						val += i10x(-prec) / 2;	/* Round */
						m = ilog10(val);
						if (m < 0) m = 0;
						if (m + prec + 3 > SZB_OUTPUT) er = "OV";
					}
				}
				if (!er) {	/* Not error condition */
					do {				/* Put decimal number */
						w = i10x(m);				/* Snip the highest digit d */
						d = val / w; val -= d * w;
						if (m == -1) *buf++ = '.';	/* Into fractional part? */
						*buf++ = '0' + d;			/* Put the digit */
					} while (--m >= -prec);			/* Output all digits specified by prec */
					if (fmt != 'f') {	/* Put exponent if needed */
						*buf++ = fmt;
						if (e < 0) {
							e = -e; *buf++ = '-';
						} else {
							*buf++ = '+';
						}
						*buf++ = '0' + e / 10;
						*buf++ = '0' + e % 10;
					}
				}
			}
			if (er) {	/* Error condition? */
				do *buf++ = *er++; while (*er);
			}
			*buf = 0;	/* Term */
		};

};

#endif
