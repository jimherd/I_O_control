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


void Task_display_control(void *p) 
{
error_codes_te   status;

    uart1_sys_init();
    gen4_uLCD_init();
    reset_4D_display();
    
    FOREVER {
        gen4_uLCD_WriteContrast(5);
        vTaskDelay(5000);
        gen4_uLCD_WriteContrast(12);
        vTaskDelay(5000);
    }
}

