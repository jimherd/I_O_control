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
#include    "system.h"


void update_task_execution_time(task_et task, uint32_t start_time, uint32_t end_time);
void print_error(int32_t port, error_codes_te sys_error);
void software_reset(void);


#endif