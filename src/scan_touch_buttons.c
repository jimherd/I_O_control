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

#define     NOS_GEN4_uLCD_FORMS     2
#define     MAX_BUTTONS_PER_FORM    4

// typedef struct  {
//     int32_t     index;
//     int32_t     value;
//     int32_t     time_high;
// } touch_button_ts;

// struct form_buttons {
//     touch_button_ts  button_data[MAX_BUTTONS_PER_FORM];
// } form_button_info[NOS_GEN4_uLCD_FORMS];


//==============================================================================
// Task code
//==============================================================================

void Task_scan_touch_buttons(void *p) 
{
error_codes_te   status;
TickType_t  xLastWakeTime;
BaseType_t  xWasDelayed;
int32_t     form_index;
uint32_t    start_time, end_time;


    xLastWakeTime = xTaskGetTickCount ();
    FOREVER {
        xWasDelayed = xTaskDelayUntil( &xLastWakeTime, TASK_SERVO_CONTROL_FREQUENCY_TICK_COUNT );
        start_time = time_us_32();
        form_index = get_active_form();
        if (form_index >= 0) {
            for (int i = 0; i < form_data[form_index].nos_buttons; i++) {
                
            }
        } else {
            // error
        }
        end_time = time_us_32();
        update_task_execution_time(TASK_SERVO_CONTROL, start_time, end_time);   
    }
}




