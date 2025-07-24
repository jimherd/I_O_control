/**
 * @file scan_touch_buttons.c
 * @author Jim Herd
 * @brief Scan display touch buttons
 * 
 * @details
 * Task runs at TASK_SCAN_TOUCH_BUTTONS_FREQUENCY (typ 5Hz) to scan touch
 * buttons on a Gen4_uLCD display.  The code will only scan the buttons
 * of the currently active form which keeps overheads to a minumum.
 * 
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

#include    "FreeRTOS.h"
#include    "timers.h"

#include    "system.h"
#include    "externs.h"
#include    "sys_routines.h"
#include    "string_IO.h"
#include    "gen4_uLCD.h"

//==============================================================================
// Task code
//==============================================================================

void Task_scan_touch_buttons(void *p) 
{
error_codes_te   status;
TickType_t  xLastWakeTime;
BaseType_t  xWasDelayed;
int32_t     new_value, old_value, current_form, result;
uint32_t    start_time, end_time;

touch_button_data_ts  *obj_pt;

    vTaskDelay(4000);
    xLastWakeTime = xTaskGetTickCount ();
    FOREVER {
        xWasDelayed = xTaskDelayUntil( &xLastWakeTime, TASK_SERVO_CONTROL_FREQUENCY_TICK_COUNT );
        start_time = time_us_32();

        current_form = get_uLCD_active_form();
        if ((current_form >= 0) && (current_form <= NOS_FORMS)) {
            for (int i = 0; i < nos_object[current_form].nos_buttons; i++) {
                obj_pt = &form_data[current_form].buttons[i];
                if (obj_pt->object_mode != OBJECT_ENABLED) {
                    continue;   // on to next button
                }
                status = gen4_uLCD_ReadObject(obj_pt->object_type, 
                                              obj_pt->global_object_id, 
                                              &result);
                if (status == OK) {
                    old_value = obj_pt->button_value;
                    new_value = result;
                    if (new_value == old_value) {   // no change to value
                        if (new_value == 1) {  // 
                            obj_pt->time_high++;
                        }
                        continue;     // next button
                    } else {  // must be a rising or falling edge
                        if (new_value == 1) {  // rising edge
                            obj_pt->button_state = NOT_PRESSED;
                            obj_pt->time_high = 0;
                        } else {               // falling edge
                            obj_pt->button_state = PRESSED;
                        }
                    }
                    // log result 
                    obj_pt->button_value = new_value;   
                }
            }
        } else {
            // error
        }
        end_time = time_us_32();
        update_task_execution_time(TASK_SCAN_TOUCH_BUTTONS, start_time, end_time);  
        vTaskDelay(TASK_SCAN_TOUCH_BUTTONS_FREQUENCY_TICK_COUNT); 
    }
}




