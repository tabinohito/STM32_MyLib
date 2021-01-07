/*
 * OLED.hpp
 *
 *  Created on: Dec 25, 2020
 *      Author: tako
 */

#ifndef SRC_OLED_HPP_
#define SRC_OLED_HPP_
#include "i2c.hpp"
#include <list>
#include <iterator>
#include <algorithm>

class OLED{
public:
	OLED() = delete;
	OLED(i2c i2c_module,uint8_t _addr = 0x3C):oled_i2c(i2c_module),addr(_addr){
		clearDisplay();
	};

	void clearDisplay(){
		oled_i2c.i2c_writeRegisterByte(addr,OLED_Command_Mode,clear);
		oled_i2c.i2c_writeRegisterByte(addr,OLED_Command_Mode,set_cursor_first_line);
		noCursor();
		noBlink();
		display();
		oled_i2c.i2c_writeRegisterByte(addr,OLED_Command_Mode,clear);
		character_cnt = 0;
	};

	void setDisplay(const uint8_t character){
		if(character_cnt < 32) {
			bool status = oled_i2c.i2c_writeRegisterByte(addr,OLED_Data_Mode,character);
			while(!status){
				status = oled_i2c.i2c_writeRegisterByte(addr,OLED_Data_Mode,character);
			}
			character_cnt++;
			stack.push_back(character);

			/*改行*/
			if(character_cnt == 16){
				oled_i2c.i2c_writeRegisterByte(addr,OLED_Command_Mode,set_cursor_second_line);
			}
		}
		else{
			clearDisplay();
			character_cnt = 17;
			std::vector<uint8_t> area;
			for(size_t i = 16;i < stack.size();i++){
				area.push_back(stack.at(i));
			}

			for(auto &i:area){
				bool status = oled_i2c.i2c_writeRegisterByte(addr,OLED_Data_Mode,i);
				while(!status){
					status = oled_i2c.i2c_writeRegisterByte(addr,OLED_Data_Mode,i);
				}
			}
			stack = area;

			oled_i2c.i2c_writeRegisterByte(addr,OLED_Command_Mode,set_cursor_second_line);

			bool status = oled_i2c.i2c_writeRegisterByte(addr,OLED_Data_Mode,character);
			while(!status){
				status = oled_i2c.i2c_writeRegisterByte(addr,OLED_Data_Mode,character);
			}
			stack.push_back(character);
		}
	};

	uint8_t getCharacter_cnt() const {return character_cnt; };

	void setNewLine(){
		if(character_cnt < 15) while(character_cnt < 15)setDisplay(' ');
		else if(character_cnt < 31) while(character_cnt < 31)setDisplay(' ');
	};

	void cursor() {
		displayMode |= I2C_DISPLAY_CURSOR_ON;
		setDisplayMode();
	};

	void noCursor() {
		displayMode &= I2C_DISPLAY_CURSOR_OFF;
		setDisplayMode();
	};

	void blink() {
		displayMode |= I2C_DISPLAY_MODE_BLINK_ON;
		setDisplayMode();
	};

	void noBlink() {
		displayMode &= I2C_DISPLAY_MODE_BLINK_OFF;
		setDisplayMode();
	};

	void display() {
		displayMode |= I2C_DISPLAY_MODE_DISPLAY_ON;
	  	setDisplayMode();
	};

	void noDisplay() {
		displayMode &= I2C_DISPLAY_MODE_DISPLAY_OFF;
		setDisplayMode();
	};

	void setDisplayMode() {
		oled_i2c.i2c_writeRegisterByte(addr, OLED_Command_Mode,displayMode|I2C_COMMAND_DISPLAY_ON_OFF);
	};

	void printf (const char* fmt, ...)
	{
		va_list arp;
		va_start(arp, fmt);
		xvprintf(fmt, arp);
		va_end(arp);
	};

private:
	i2c oled_i2c;
	const uint8_t addr;
	uint8_t displayMode =  0b00000000;
	size_t character_cnt = 0;
	static constexpr auto OLED_Command_Mode = 0x00;
	static constexpr auto OLED_Data_Mode = 0x40;
	static constexpr auto clear = 0x01;
	static constexpr auto set_cursor_first_line = 0x02;
	static constexpr auto set_cursor_second_line = 0x20 + 0x80;
	static constexpr auto I2C_COMMAND_DISPLAY_ON_OFF = 0b00001000;
	static constexpr auto I2C_DISPLAY_MODE_BLINK_ON = 0b00000001;
	static constexpr auto I2C_DISPLAY_MODE_BLINK_OFF = 0b11111110;
	static constexpr auto I2C_DISPLAY_CURSOR_ON = 0b00000010;
	static constexpr auto I2C_DISPLAY_CURSOR_OFF = 0b11111101;
	static constexpr auto I2C_DISPLAY_MODE_DISPLAY_ON = 0b00000100;
	static constexpr auto I2C_DISPLAY_MODE_DISPLAY_OFF = 0b11111011;
	std::vector<uint8_t> stack;

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

	void xputc (char c){
		if (_CR_CRLF && c == '\n') setNewLine();

		setDisplay((unsigned char)c);
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
};



#endif /* SRC_OLED_HPP_ */
