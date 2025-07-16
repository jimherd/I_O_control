/**
 * @file display.c
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

bool    display_OK;

//==============================================================================
// Task code
//==============================================================================

void Task_display_control(void *p) 
{
error_codes_te   status;

    uart1_sys_init();
    display_OK = true;
    status = gen4_uLCD_init();
    if (status != OK) {
        display_OK = false;
    }
    if (display_OK == true) {
        status = change_uLCD_form(GEN4_uLCD_FORM0);  // change to form 1
        if (status != OK) {
            display_OK = false;
        }
        
        status = uLCD_printf(GEN4_uLCD_FORM0, GEN4_uLCD_STRING0, "V%d.%d", MAJOR_VERSION, MINOR_VERSION);
        //status = uLCD_WriteString(FORM0, STRING0, "New string");
    }
    
    FOREVER {
        if (display_OK == true){
            gen4_uLCD_WriteContrast(5);
            vTaskDelay(2000);
            gen4_uLCD_WriteContrast(12);
            vTaskDelay(2000);
        } else {
            vTaskDelay(10000);
        }
    }
}

