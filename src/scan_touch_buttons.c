/**
 * @file scan_touch_buttons.c
 * @author Jim Herd
 * @brief Control 4D Systems touch screen display
 */

#include    <stdio.h>
#include    <string.h>
#include    <stdarg.h>

#include    "pico/stdlib.h"
#include    "pico/binary_info.h"

#include    "hardware/gpio.h"
#include    "hardware/uart.h"
#include    "hardware/irq.h"

#include    "FreeRTOS.h"
#include    "event_groups.h"
#include    "timers.h"
#include    "queue.h"

#include    "system.h"
#include    "externs.h"
#include    "sys_routines.h"
#include    "string_IO.h"
#include    "min_printf.h"
#include    "gen4_uLCD.h"

//==============================================================================
// Task code
//==============================================================================

void Task_scan_touch_buttons(void *p) 
{
error_codes_te   status;
TickType_t  xLastWakeTime;
BaseType_t  xWasDelayed;
int32_t     current_form, result;
uint32_t    start_time, end_time;


    xLastWakeTime = xTaskGetTickCount ();
    FOREVER {
        xWasDelayed = xTaskDelayUntil( &xLastWakeTime, TASK_SERVO_CONTROL_FREQUENCY_TICK_COUNT );
        start_time = time_us_32();
        current_form = get_active_form();
        if (current_form >= 0) {
            for (int i = 0; i < GEN4_uLCD_MAX_NOS_BUTTONS; i++) {
                if (button_data[i].enable == false) {
                    continue;
                }
                if (button_data[i].form != current_form) {
                    continue;
                }
                status = gen4_uLCD_ReadObject(GEN4_uLCD_OBJ_IBUTTONE, i, &result);
                if (status == OK) {
                    button_data[i].button_data = result;
                    if( result == 1) {
                        button_data[i].time_high++;
                    } else {
                        button_data[i].time_high = 0;
                    }
                }
            }
        } else {
            // error
        }
        end_time = time_us_32();
        update_task_execution_time(TASK_SERVO_CONTROL, start_time, end_time);   
    }
}




