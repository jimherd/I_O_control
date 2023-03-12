//***************************************************************************
// sys_routines.cpp :  useful routines
//***************************************************************************
//
// 1. faster and lower overhead replacments for the following routines
//      a. atoi
//      b. atof
//      c. itoa
// 2. send strings through queue to HLLControl system
//

#include "system.h"
#include "sys_routines.h"

#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"


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

char* int_to_ASCII(int32_t num, char* str) 
{ 
    int32_t i = 0; 
    bool isNegative = false; 
  
    /* Handle 0 explicitely, otherwise empty string is printed for 0 */
    if (num == 0) { 
        str[i++] = '0'; 
        str[i] = ' '; 
        return str; 
    } 
  
    if (num < 0) { 
        isNegative = true;
		str[i++] = '-';		
        num = -num; 
    } 
  
    // Process individual digits 
    while (num != 0) { 
        int32_t rem = num % 10; 
        str[i++] = rem + '0'; 
        num = num/10; 
    } 
  
    str[i] = ' '; // Append string terminator 
    return str; 
} 



//==============================================================================
/**
 * @brief Calculate task execution time and record
 * @note  Check for new low/high values and record
 * @param task 
 * @param start_time    units of uS
 * @param end_time      units of uS
 */
void update_task_execution_time(task_et task, uint32_t start_time, uint32_t end_time) 
{
uint32_t delta_time;

    if (end_time > start_time) {
        delta_time = end_time - start_time;
    } else {
        delta_time = (UINT32_MAX - start_time) + end_time + 1;
    }

    task_data[task].last_exec_time = delta_time;

    if (delta_time < task_data[task].lowest_exec_time) {
        task_data[task].lowest_exec_time = delta_time;
    }
    
    if (delta_time > task_data[task].highest_exec_time) {
        task_data[task].highest_exec_time = delta_time;
    }
}
