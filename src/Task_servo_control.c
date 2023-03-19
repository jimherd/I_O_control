/**
 * @file Task_servo_control.c
 * @author Jim Herd
 * @brief Manage RC servo motors
 */
#include <stdio.h>
#include <stdlib.h>

#include "system.h"
#include "uart_IO.h"
#include "sys_routines.h"
#include "PCA9685.h"

#include  "Pico_IO.h"

#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "FreeRTOS.h"

void Task_servo_control(void *p) {

    init_PCA9685_servo_IO();

    FOREVER {
        
        vTaskDelay(20000);
    }
}

