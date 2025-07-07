/**
 * @file scan_touch_buttons.c
 * @author Jim Herd
 * @brief Scan display touch buttons
 * 
 * @details
 * Task runs at TASK_SCAN_TOUCH_BUTTONS_FREQUENCY (typ 10Hz) to scan touch
 * buttons on a Gen4_uLCD display.  The code will only scan the buttons
 * of the currently active form which keeps overheads to a minumum.
 * System also measures the duration of a press of a button.  This allows
 * the detection of a long press, which can be used to enable "hidden" modes,
 * eg test modes.
 */

#include    <stdio.h>
#include    <string.h>
#include    <stdarg.h>

#include    "pico/stdlib.h"
#include    "pico/binary_info.h"

#include    "hardware/gpio.h"
#include    "hardware/uart.h"
// #include    "hardware/irq.h"

#include    "FreeRTOS.h"
// #include    "event_groups.h"
#include    "timers.h"
// #include    "queue.h"

#include    "system.h"
#include    "externs.h"
#include    "sys_routines.h"
#include    "string_IO.h"
// #include    "min_printf.h"
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

    vTaskDelay(4000);
    xLastWakeTime = xTaskGetTickCount ();
    FOREVER {
        xWasDelayed = xTaskDelayUntil( &xLastWakeTime, TASK_SERVO_CONTROL_FREQUENCY_TICK_COUNT );
        start_time = time_us_32();
        current_form = get_active_form();
        if ((current_form >= 0) && (current_form <= NOS_FORMS)) {
            for (int i = 0; i < NOS_BUTTONS; i++) {
                if (button_data[i].enable == false) {
                    continue;
                }
                // if (button_data[i].form != current_form) {
                //     continue;
                // }
                // status = gen4_uLCD_ReadObject(button_data[i].button_type, button_data[i].button_id, &result);
                if (status == OK) {
                    button_data[i].button_value = result;
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
        update_task_execution_time(TASK_SCAN_TOUCH_BUTTONS, start_time, end_time);   
    }
}




