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

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "externs.h"
#include "system.h"
#include "uart_IO.h"
#include "string_IO.h"
#include "gen4_uLCD.h"

#include "pico/stdlib.h"

//==============================================================================
/**
 * @brief 
 * 
 * @param &string_buffer *buff_pt 
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
 * @param &string_buffer *buff_pt
 * @param string 
 * @param width 
 * @param pad 
 * @return int32_t 
 */
static void prints(struct string_buffer *buff_pt, const char *string)
{
	add_string_to_char_buffer(buff_pt, string);
}

// static void prints(struct string_buffer *buff_pt *&string_buffer *buff_pt, const char *string, int32_t width, int32_t pad)
// {
// int32_t  len, padchar = SPACE;
// const char*	ptr;

//     if (width > 0) {
//         len = 0;
//         for (ptr = string; *ptr; ++ptr) {
// 			++len;
// 		}
//         if (len >= width) {
//             width = 0;
// 		} else {
//             width -= len;
// 		}
//         if (pad & PAD_ZERO) {
// 			padchar = CHAR_0;
//     	}
// 		if (!(pad & PAD_RIGHT)) {
// 			for (; width > 0; --width) {
// 				printchar(&string_buffer *buff_pt, padchar);
// 			}
//     	}
// 		for (; *string; ++string) {
// 			printchar(&string_buffer *buff_pt, *string);
// 		}
// 		for (; width > 0; --width) {
// 			printchar(&string_buffer *buff_pt, padchar);
// 		}
// 		return;
// 	}
// }

//==============================================================================
#define PRINT_BUF_LEN 12
/**
 * @brief   print various integer formats
 * 
 * @param &string_buffer *buff_pt 		buffer to receive string
 * @param i 			number to be converted to a string
 * @param base			number base (typ 10, 16)
 * @param signed_numb	number can be signed
 * @param width 
 * @param pad 
 * @param letbase 		'a'-> lowercase ; 'A' -> uppercase
 */

static void printi(struct string_buffer *buff_pt, int32_t int_value, uint32_t base, uint32_t letter_case) 
{
	add_int_to_char_buffer(buff_pt, int_value, base, letter_case);
}

// static void printi(struct string_buffer *buff_pt *&string_buffer *buff_pt, int32_t i, int32_t base, bool signed_numb, int32_t width, int32_t pad, int32_t letbase)
// {
// char 	  temp_buf[PRINT_BUF_LEN];
// char 	  *str_pt;
// int32_t   remainder; 
// bool      neg;
// uint32_t  number = i;

// 	neg = false;
//     if (i == 0) {
// 		temp_buf[0] = CHAR_0;
// 		temp_buf[1] = STRING_NULL;
// 		prints (&string_buffer *buff_pt, temp_buf /* ,width, pad */);
// 		return;
// 	}

// // if (signed_numb && b == 10 && i < 0) {
// 	if ((signed_numb == true) && (base == 10) && (i < 0)) {
// 		neg = true;
// 		number = -i;
// 	}
//     str_pt = temp_buf + PRINT_BUF_LEN - 1;
//     *str_pt = STRING_NULL;

// 	while (number > 0) {
// 		remainder = number % base;
// 		if( remainder >= 10 ) {
// 			remainder += letbase - CHAR_0 - 10;
// 		}
// 		*--str_pt = remainder + CHAR_0;
// 		number /= base;
// 	}

// 	if (neg == true) {
// 		if( width && (pad & PAD_ZERO) ) {
// 			printchar (&string_buffer *buff_pt, MINUS);
// 			--width;
// 		} else {
// 			*--str_pt = MINUS;
// 		}
// 	}
// 	prints (&string_buffer *buff_pt, str_pt /* ,width, pad */);
// 	return;
// }

//==============================================================================
/**
 * @brief Minimal sprintf routine (s, d, x, X, u, c)
 * 
 * @param &string_buffer *buff_pt 	pointer to destination cgaracter buffer
 * @param format 	standard printf format string
 * @param vargs 	access to variable list of parameters
 */
void min_format_string(struct string_buffer *buff_pt, const char *format, va_list vargs)
{
int32_t width, pad;

	for (; *format != 0; ++format) {
        if (*format == PERCENT) {
			++format;
			width = pad = 0;
			if (*format == STRING_NULL) {
				break;
			}
			if (*format == PERCENT) {
				goto out;
			}
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
				prints (buff_pt, s?s:"(null)" /*, width, pad */);
				continue;
			}
			if( *format == 'd' ) {
				printi (buff_pt, (int32_t)va_arg(vargs, int32_t), BASE_10, LOWER_CASE);
				continue;
			}
			if( *format == 'x' ) {
				printi (buff_pt, (int32_t)va_arg(vargs, int32_t), BASE_16, LOWER_CASE);
				continue;
			}
			if( *format == 'X' ) {
				printi (buff_pt, (int32_t)va_arg(vargs, int32_t), BASE_16, UPPER_CASE);
				continue;
			}
			if( *format == 'u' ) {
			//	printi (&string_buffer *buff_pt, (int32_t)va_arg(vargs, int32_t), 10, false, width, pad, 'a');
				continue;
			}
			if( *format == 'c' ) {
				printchar(buff_pt, (char)va_arg(vargs, int32_t));
				continue;
			}
		}
		else {
	out:
			printchar (buff_pt, *format);
		}
	}
	printchar(buff_pt, STRING_NULL);
	return;
}

//==============================================================================
//==============================================================================
//
// Usage - micro_printf("Hello %s, number is %d\n", "World", 42);

// void micro_printf(const char *format, ...) {
//     va_list args;  // Declare a va_list variable to manage the variable arguments

//     // Initialize the va_list 'args' to start at the argument after 'format'
//     va_start(args, format);

//     while (*format)  // Loop through the format string
//     {
//         // If a format specifier is encountered
//         if (*format == '%') {
//             format++;
//             if (*format == 'd') {
//                 // Fetch the next argument as an integer and print it
//                 printf("%d", va_arg(args, int));
//             } else if (*format == 's') {
//                 // Fetch the next argument as a string and print it
//                 printf("%s", va_arg(args, char *));
//             }
//         } else {
//             // Print regular characters
//             putchar(*format);
//         }
//         format++;  // Move to the next character
//     }
//     // Cleanup the va_list 'args' after processing
//     va_end(args);
// }

//==============================================================================
/**
 * @brief 
 * 
 * @param format  string with characters, %d, and %s. 
 * @param ...     string and integer parameters
 */

void min_sprintf(struct string_buffer *buff_pt, char *format, va_list args)
{
   int32_t width, pad;

	for (; *format != 0; ++format) {
        if (*format == PERCENT) {
			++format;
			if (*format == STRING_NULL) {
				break;
			}
			if( *format == 's' ) {
				char *s = (char *)va_arg(args, int32_t);
				prints (buff_pt, s?s:"(null)" /*, width, pad */);
				continue;
			}
			if( *format == 'd' ) {
				printi (buff_pt, (int32_t)va_arg(args, int32_t), BASE_10, LOWER_CASE);
				continue;
			}
			if( *format == 'c' ) {
				printchar(buff_pt, (char)va_arg(args, int32_t));
				continue;
			}
		}
		else {
	out:
			printchar (buff_pt, *format);
		}
	}
	printchar(buff_pt, STRING_NULL);
	return;
} 


//==============================================================================

error_codes_te uLCD_printf(uint32_t form, uint32_t local_index, const char* format, ...)
{
struct string_buffer buff;
error_codes_te  status;

	if (form != get_uLCD_active_form()) {
		return GEN4_uLCD_STRING_FORM_INACTIVE;
	}
	va_list  args;
	init_string_buffer(&buff);
	va_start(args, format);
	min_format_string(&buff, format, args);
	va_end(args);
	// write buffer to LCD
	status = gen4_uLCD_WriteString(form_data[form].strings[local_index].global_object_id, 
		                  &buff.buffer[0]);
	if (status == OK) {  // copy to 'form_data' structure
		strcpy(form_data[form].strings[local_index].string, &buff.buffer[0]);
	}
	return status;
}
