/**
 * @file sys_routines.h
 * @author Jim Herd 
 * @brief Some general purpose routines
 * @version 0.1
 * @date 2023-03-09
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef __SYS_ROUTINES_H__
#define __SYS_ROUTINES_H__

#include    "pico/stdlib.h"


int32_t ASCII_to_int(char *str);
float ASCII_to_float(const char *char_pt);
char* int_to_ASCII(int32_t num, char* str);
void update_task_execution_time(task_et task, uint32_t start_time, uint32_t end_time);

#endif