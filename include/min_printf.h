#ifndef __MIN_PRINTF_H__
#define __MIN_PRINTF_H__

#include    "pico/stdlib.h"

static void prints(struct string_buffer *buff_pt, const char *string);
static void printi(struct string_buffer *buff_pt, int32_t int_value, uint32_t base, uint32_t letter_case);

void min_format_string(struct string_buffer *buff_pt, const char *format, va_list vargs);
void min_sprintf(const char *format, ...);


#endif