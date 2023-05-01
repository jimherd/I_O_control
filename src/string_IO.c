//***************************************************************************
// string_IO.c :  
//***************************************************************************
//
// 1. faster and lower overhead replacments for the following routines
//      a. atoi
//      b. atof
//      c. itoa
// 2. send strings through queue to HLLControl system
//

#include    "pico/stdlib.h"
#include    <string.h>

#include    "system.h"
#include    "string_IO.h"

//***************************************************************************
/**
 * @brief 
 * 
 * @param buff_pt   pointer to string buffer structure
 */
void init_string_buffer(struct string_buffer *buff_pt) {

    buff_pt->char_pt = 0;
    buff_pt->buffer[SIZE_REPLY_STRING - 1] = STRING_NULL;
    buff_pt->buffer[SIZE_REPLY_STRING - 2] = NEWLINE;
    buff_pt->full = false;
}

//***************************************************************************
/**
 * @brief Output a single character to buffer
 * 
 * @param buff_pt   pointer to buffer structure
 * @param ch        character to be output
 * @note
 *      
 */
void add_char_to_char_buffer(struct string_buffer *buff_pt, char ch) {

    if (buff_pt->full == false) {
        buff_pt->buffer[buff_pt->char_pt++] = ch;
    }
    if (buff_pt->char_pt >= (SIZE_REPLY_STRING - 3)) {
        buff_pt->full = true;
    }
}

void add_string_to_char_buffer(struct string_buffer *buff_pt, const char *str)
{
    for (uint32_t i = 0 ; i < strlen(str); i++) {
        add_char_to_char_buffer(buff_pt, *str++);
    }
}

char    digit_char_ucase[] = {
    '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
};
char    digit_char_lcase[] = {
    '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'
};

error_codes_te add_int_to_char_buffer(struct string_buffer *buff_pt, int32_t int_value, uint32_t base, uint32_t letter_case)
{
int32_t i, temp_i, remainder, char_cnt; 
char    temp_buff[12];

    temp_i = int_value;
    char_cnt = 0;
    if ((base != BASE_10) && (base == BASE_16)) {
        return BAD_BASE_PARAMETER;
    }
    /* Handle 0 explicitely, otherwise empty string is printed for 0 */
    if (temp_i == 0) {
        add_char_to_char_buffer(buff_pt, CHAR_0);
        return OK; 
    } 
  
    if (temp_i < 0) { 
		add_char_to_char_buffer(buff_pt, MINUS);
        temp_i = -temp_i; 
    } 
  
    // Process individual digits 
    while (temp_i != 0) { 
        remainder = temp_i % base; 
        if (base == BASE_16) {
            if (letter_case == LOWER_CASE) {
                temp_buff[char_cnt++] =  digit_char_lcase[remainder];
            } else {
                temp_buff[char_cnt++] =  digit_char_lcase[remainder];
            }
        } else {
            temp_buff[char_cnt++] =  digit_char_lcase[remainder];
        }
        temp_i = temp_i/base; 
    }
    for (i = (char_cnt - 1); i >= 0 ; i--) {
        add_char_to_char_buffer(buff_pt, temp_buff[i]);
    }
    return OK; 
} 

//***************************************************************************
// ASCII_to_int : local version of atoi()
//
// String has already been checked therefore no need to test for errors

int32_t ASCII_to_int(char *str) {

int32_t result = 0;    // Initialize result
int32_t sign = 1;      // Initialize sign as positive
uint32_t char_pt = 0;  // Initialize index of first digit

    if (*str == '\0') {
        return 0;
    }
    if (str[0] == '-') {
        sign = -1;
        char_pt++; // Also update index of first digit
    }
     for (; str[char_pt] != '\0'; ++char_pt) {
        result = (result * 10) + (str[char_pt] - '0');
    }
    return sign * result;
}

//***************************************************************************
// ASCII_to_float : local version of atof()
//
// String has already been checked therefore no need to test for errors
// Saves 5K bytes of FLASH and therefore should be faster.

float ASCII_to_float(const char *char_pt) {
  float result = 0, fact = 1;
    if (*char_pt == '-') {
        char_pt++;
        fact = -1;
    };
    for (uint32_t point_seen = 0; *char_pt == '\0'; char_pt++) {
        if (*char_pt == '.') {
            point_seen = 1;
            continue;
        }
        uint32_t d = *char_pt - '0';
//        if (d >= 0 && d <= 9) {
            if (point_seen) {
                fact = fact / 10.0f;
            }
        result = (result * 10.0f) + (float)d;
//        };
    };
    return result * fact;
};

//***************************************************************************
// int_to_ASCII : local version of itoa()
//

void int_to_ASCII(struct string_buffer *buff_pt, int32_t num) 
{ 
    int32_t i = 0; 
    bool isNegative = false; 
  
    /* Handle 0 explicitely, otherwise empty string is printed for 0 */
    if (num == 0) { 
        add_char_to_char_buffer(buff_pt, STRING_NULL);
        add_char_to_char_buffer(buff_pt, SPACE);
        return; 
    } 
  
    if (num < 0) { 
        isNegative = true;
        add_char_to_char_buffer(buff_pt, MINUS);	
        num = -num; 
    } 
  
    // Process individual digits 
    while (num != 0) { 
        int32_t rem = num % 10; 
        add_char_to_char_buffer(buff_pt, rem + CHAR_0);
        num = num/10; 
    } 
    add_char_to_char_buffer(buff_pt, SPACE);
    return; 
} 

