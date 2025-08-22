/**
 * @file Task_blink_LED.c
 * @author Jim Herd
 * @brief Flash Pico board LED
 */
#include <stdio.h>
#include <stdlib.h>

#include "system.h"
#include "sys_routines.h"

#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "FreeRTOS.h"

void Task_blink(void *p) 
{

    gpio_init(BLINK_PIN);
    gpio_set_dir(BLINK_PIN, GPIO_OUT);
    
    FOREVER {
        gpio_put(BLINK_PIN, 1);
        gpio_put(LOG_PIN, 1);
        vTaskDelay(1000);
        gpio_put(BLINK_PIN, 0);
        gpio_put(LOG_PIN, 0);
        vTaskDelay(1000);
    }
}


