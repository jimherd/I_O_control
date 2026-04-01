/**
 * @file    Task_scan_push_buttons.c
 * @author  Jim Herd
 * @brief   REad and debounce user I/O push buttons
 */

#include <string.h>
#include <assert.h>

#include "system.h"
#include "Pico_IO.h"
#include "sys_routines.h"
#include "externs.h"


#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/divider.h"
#include "hardware/pio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

//==============================================================================
// Main task routine
//==============================================================================
//

void Task_scan_push_buttons(void *p) 
{
TickType_t  xLastWakeTime;
BaseType_t  xWasDelayed;
uint8_t     index;
uint32_t    start_time, end_time;

//==============================================================================
// Task code
//==============================================================================
    xLastWakeTime = xTaskGetTickCount ();
    FOREVER {
        xWasDelayed = xTaskDelayUntil( &xLastWakeTime, TASK_SCAN_PUSH_BUTTONS_FREQUENCY );
        start_time = time_us_32();
        

        end_time = time_us_32();
        update_task_execution_time(TASK_SCAN_PUSH_BUTTONS, start_time, end_time);
    }
}