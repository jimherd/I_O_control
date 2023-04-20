/*
    Copyright 2001-2021 Georges Menie
    https://www.menie.org/georges/embedded/small_printf_source_code.html

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*
	putchar is the only external dependency for this file,
	if you have a working putchar, just remove the following
	define. If the function should be called something else,
	replace outbyte(c) by your own function call.
*/

#include <stdio.h>
#include <stdarg.h>

#include "system.h"
#include "uart_IO.h"
#include "string_IO.h"


#include "pico/stdlib.h"

//==============================================================================
/**
 * @brief 
 * 
 * @param buff_pt 
 * @param character 
 */
static inline void printchar(struct string_buffer *buff_pt, char character)
{
	add_char_to_char_buffer(buff_pt, character);
}

//==============================================================================
#define PAD_RIGHT 1
#define PAD_ZERO 2
/**
 * @brief 
 * 
 * @param buff_pt 
 * @param string 
 * @param width 
 * @param pad 
 * @return int32_t 
 */
static int32_t prints(struct string_buffer *buff_pt, const char *string, int32_t width, int32_t pad)
{
int32_t pc = 0, padchar = SPACE;

    if (width > 0) {
        int32_t len = 0;
        const char *ptr;
        for (ptr = string; *ptr; ++ptr) ++len;
        if (len >= width)
            width = 0;
        else
            width -= len;
        if (pad & PAD_ZERO) padchar = CHAR_0;
    }
    if (!(pad & PAD_RIGHT)) {
        for (; width > 0; --width) {
            printchar(buff_pt, padchar);
            ++pc;
        }
    }
    for (; *string; ++string) {
        printchar(buff_pt, *string);
        ++pc;
    }
    for (; width > 0; --width) {
        printchar(buff_pt, padchar);
        ++pc;
    }
    return pc;
}

//==============================================================================
/* the following should be enough for 32 bit integers */
#define PRINT_BUF_LEN 12
/**
 * @brief 
 * 
 * @param buff_pt 
 * @param i 
 * @param b 
 * @param sg 
 * @param width 
 * @param pad 
 * @param letbase 
 * @return int32_t 
 */
static int32_t printi(struct string_buffer *buff_pt, int32_t i, int32_t b, int32_t sg, int32_t width, int32_t pad, int32_t letbase)
{
char print_buf[PRINT_BUF_LEN];
char *s;
int32_t t, neg = 0, pc = 0;
uint32_t u = i;

    if (i == 0) {
		print_buf[0] = CHAR_0;
		print_buf[1] = STRING_NULL;
		return prints (buff_pt, print_buf, width, pad);
	}

	if (sg && b == 10 && i < 0) {
		neg = 1;
		u = -i;
	}

	s = print_buf + PRINT_BUF_LEN-1;
	*s = STRING_NULL;

	while (u) {
		t = u % b;
		if( t >= 10 )
			t += letbase - CHAR_0 - 10;
		*--s = t + CHAR_0;
		u /= b;
	}

	if (neg) {
		if( width && (pad & PAD_ZERO) ) {
			printchar (buff_pt, '-');
			++pc;
			--width;
		}
		else {
			*--s = '-';
		}
	}

	return pc + prints (buff_pt, s, width, pad);
}


//==============================================================================
/**
 * @brief 
 * 
 * @param buff_pt 
 * @param varg 
 * @return int32_t 
 */
int32_t min_sprintf(struct string_buffer *buff_pt, const char *format, va_list vargs)
{
int32_t width, pad;
int32_t pc = 0;
char scr[2];

	for (; *format != 0; ++format) {
                if (*format == '%') {
			++format;
			width = pad = 0;
			if (*format == STRING_NULL) break;
			if (*format == '%') goto out;
			if (*format == MINUS) {
				++format;
				pad = PAD_RIGHT;
			}
			while (*format == CHAR_0) {
				++format;
				pad |= PAD_ZERO;
			}
			for ( ; ((*format >= CHAR_0) && (*format <= '9')) ; ++format) {
				width *= 10;
				width += *format - CHAR_0;
			}
			if( *format == 's' ) {
				 char *s = (char *)va_arg(vargs, int32_t);
				pc += prints (buff_pt, s?s:"(null)", width, pad);
				continue;
			}
			if( *format == 'd' ) {
				pc += printi (buff_pt, (int32_t)va_arg(vargs, int32_t), 10, 1, width, pad, 'a');
				continue;
			}
			if( *format == 'x' ) {
				pc += printi (buff_pt, (int32_t)va_arg(vargs, int32_t), 16, 0, width, pad, 'a');
				continue;
			}
			if( *format == 'X' ) {
				pc += printi (buff_pt, (int32_t)va_arg(vargs, int32_t), 16, 0, width, pad, 'A');
				continue;
			}
			if( *format == 'u' ) {
				pc += printi (buff_pt, (int32_t)va_arg(vargs, int32_t), 10, 0, width, pad, 'a');
				continue;
			}
			if( *format == 'c' ) {
				/* char are converted to int then pushed on the stack */
				// scr[0] = *varg++;
				// scr[1] = STRING_NULL;
				// pc += prints (buff_pt, scr, width, pad);
				printchar(buff_pt, (char)va_arg(vargs, int32_t));
				continue;
			}
		}
		else {
		out:
			printchar (buff_pt, *format);
			++pc;
		}
	}
	printchar(buff_pt, STRING_NULL);
	return pc;
}


