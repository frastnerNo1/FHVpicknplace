/******************************************************************************
* This file is for rscanf()
* The rscanf() is a simple scanf() and only can read input strings with %s,%d,%c.
* It is based on a function published by Kumar Sourav under THE MIT LICENSE, see:
* https://github.com/kumarsourav/Simple-Scanf
* IMPORTANT: TO BE USED ONLY TOGETHER WITH rprintf()!
* IMPORTANT: In case of %c and %d you have to pass the address (&...)!
* IMPORTANT: In case of %d you need a 32 Bit variable (long or similar)!
* BEFORE USE rprintf_init() HAS TO BE CALLED!
*******************************************************************************/
#include <asf.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include "rscanf.h"

#define MAXLN	255
#define SPACE	"\t\n\f\r " 

extern struct usart_module my_usart_instance;

uint8_t *check(uint8_t *bptr);
uint8_t *rgets (uint8_t *buff, uint8_t length);
uint8_t *fillbuff(uint8_t *buff);

uint8_t *check(uint8_t *bptr)
{
	while (*bptr != '\n') {
		if (!isspace(*bptr))
			return bptr;
		++bptr;	
	}
	return bptr;
}

uint8_t *rgets (uint8_t *buff, uint8_t length) {
	uint8_t myChar, i = 0;
	do {
		usart_serial_getchar(&my_usart_instance, &myChar);
		*(buff+i) = myChar;
		i++;
	} while (!((myChar == '\r')||(myChar == '\n')));
	return buff;
}



uint8_t *fillbuff(uint8_t *buff)
{
	rgets(buff, MAXLN);
	return buff;
}

uint16_t rscanf(const uint8_t *fmt, ...)
{
	volatile uint8_t buff[MAXLN], *str, *bptr;
	uint16_t len = 0, count = 0, space = 0;
	uint32_t *var;
	uint8_t tmp[MAXLN];
	va_list ap;
	va_start(ap, fmt);
	bptr = fillbuff(buff);
	while (*fmt) {
		len = 0;
		while (*fmt && isspace(*fmt)) {
			++fmt;
			++space;
		}
		if (!*fmt)
			break;
		if (*fmt == 's') {
			bptr = check(bptr);
			if (*bptr == '\n') {
				bptr = fillbuff(buff);
				continue;
			}
			str = va_arg(ap, uint8_t *);
			len = strcspn((char*)bptr, SPACE);
			strncpy((char*)str, (char*)bptr, len);
			bptr += len;
			str += len; //HP: to add a \0 at the end in all cases
			*str=0; //HP: to add a \0 at the end in all cases
			++count;
			space = 0;
		} else if (*fmt == 'd') { 
			bptr = check(bptr);
			if (*bptr == '\n') {
				bptr = fillbuff(buff);
				continue;
			}
			var = va_arg(ap, uint32_t *);
			len = strcspn((char*)bptr, SPACE);
			strncpy((char*)tmp, (char*)bptr, len);
			*var = atoi((char*)tmp);
			bptr += len;
			++count;
			space = 0;
		} else if (*fmt == 'c') {
			len = 1;
			if (space > 0) {
				bptr = check(bptr);
				if (*bptr == '\n') {
					bptr = fillbuff(buff);
					continue;
				}
			}
			str = va_arg(ap, uint8_t *);
			strncpy((char*)str, (char*)bptr, len);
			if (*bptr == '\n') {
				bptr = fillbuff(buff);
				len = 0;
			}		
			bptr += len;
			++count;
			space = 0;
		} else
			;
		++fmt;
	}
	va_end(ap);
	return count;
}