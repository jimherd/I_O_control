//***************************************************************************
// sys_routines.cpp :  useful routines
//***************************************************************************
//
//

#include "system.h"
#include "externs.h"
#include "sys_routines.h"
#include "uart_IO.h"


#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

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
uint32_t sm_delay;

    if (end_time > start_time) {
        sm_delay = end_time - start_time;
    } else {
        sm_delay = (UINT32_MAX - start_time) + end_time + 1;
    }

    task_data[task].last_exec_time = sm_delay;

    if (sm_delay < task_data[task].lowest_exec_time) {
        task_data[task].lowest_exec_time = sm_delay;
    }
    
    if (sm_delay > task_data[task].highest_exec_time) {
        task_data[task].highest_exec_time = sm_delay;
    }
}

//==============================================================================
/**
 * @brief 
 * 
 * @param port 
 * @param sys_error 
 */
void print_error(int32_t port, error_codes_te sys_error)
{
    print_string("%d %d\n", port, sys_error);
}

