/**
 * @file string_IO.h
 * @author Jim Herd 
 * @brief 
 * @date 2023-11-08
 * 
 * 
 */
#ifndef __STRING_IO_H__
#define __STRING_IO_H__

#include    "pico/stdlib.h"

#define     SIZE_REPLY_STRING   128

struct string_buffer {
    char        buffer[SIZE_REPLY_STRING];
    uint32_t    char_pt;
    bool        full;
};

//==============================================================================
// Function prototypes
//==============================================================================

void init_string_buffer(struct string_buffer *buff_pt);
void add_char_to_char_buffer(struct string_buffer *buff_pt, char ch);
void add_string_to_char_buffer(struct string_buffer *buff_pt, const char *str);
error_codes_te add_int_to_char_buffer(struct string_buffer *buff_pt, int32_t int_value, uint32_t base, uint32_t letter_case);

int32_t ASCII_to_int(char *str);
float ASCII_to_float(const char *char_pt);


#endif /* __STRING_IO_H__ */