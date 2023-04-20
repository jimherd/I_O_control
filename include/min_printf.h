#ifndef __MIN_PRINTF_H__
#define __MIN_PRINTF_H__

#include    "pico/stdlib.h"

//int32_t min_sprintf(struct string_buffer *buff_pt, const char *format, ...);

int32_t min_sprintf(struct string_buffer *buff_pt, const char *format, va_list vargs);

//int32_t min_sprintf(struct string_buffer *buff_pt, const char *format, ...);


#endif