/**
 * @file    Task_neopixel.c
 * @author  Jim Herd
 * @brief   read Robokid sensors and set neopixel LEDs
 */

#include <string.h>
#include <assert.h>

#include "system.h"
#include "Pico_IO.h"
#include "sys_routines.h"
#include "externs.h"
#include "neopixel.h"

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/divider.h"
#include "hardware/pio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

#include "neopixel.pio.h"

//==============================================================================
// Constant definitions
//==============================================================================

const struct colour {
    uint8_t   red;
    uint8_t   green;
    uint8_t   blue;
} rainbow_col[9] = {
        {255, 0 , 0},     // Red
        {255, 127, 0},    // Orange
        {255, 255, 0},    // Yellow
        {0, 255, 0},      // Green
        {0, 0, 255},      // Blue
        {75, 0, 130},     // Indigo
        {148, 0, 211},    // Violet
        {255, 255,255},   // White
        {0, 0 , 0},       // Black
};

//==============================================================================
// Main task routine
//==============================================================================
//

void Task_write_neopixels(void *p) 
{
TickType_t  xLastWakeTime;
BaseType_t  xWasDelayed;
uint8_t     index;
uint32_t    start_time, end_time;

// 
// Initialise buffer info, state machine (sm), and DMA channel
//
    init_neopixel_buffer();
    init_neopixel_sm();
    init_neopixel_DMA(NEOPIXEL_PIO_UNIT, NEOPIXEL_STATE_MACHINE);
//
// Some test data
//
        load_pixel(1, urgb_u32(0x1f, 0, 0));       // Red
        load_pixel(2, urgb_u32(0, 0x1f, 0));       // Green
        load_pixel(0, urgb_u32(0, 0, 0x1f));       // Blue
        load_pixel(3, urgb_u32(0x10, 0x10, 0x10)); // 

//==============================================================================
// Task code
//==============================================================================
    xLastWakeTime = xTaskGetTickCount ();
    FOREVER {
        xWasDelayed = xTaskDelayUntil( &xLastWakeTime, TASK_NEOPIXELS_FREQUENCY_TICK_COUNT );
        start_time = time_us_32();
//
// DMA is re-triggered on each call to the neopixel task.
// Allows timed effects for lights
//
        trigger_neopixel_dma();
        vTaskDelay(5);   /* delay before modifying the neopixels buffer values */

// Process LED data

        for (index = 0 ; index < NOS_NEOPIXELS ; index++) { 
            switch (neopixel_data[index].command) {
                case N_CMD_ON:
                    neopixel_data[index].current_colour = neopixel_data[index].on_colour;
                    break;
                case N_CMD_OFF:
                    neopixel_data[index].current_colour = neopixel_data[index].off_colour;
                    break;
                case N_CMD_FLASH:
                    if (neopixel_data[index].state == N_FLASH_OFF) {
                        neopixel_data[index].flash_off_counter--;
                        if (neopixel_data[index].flash_off_counter <= 0) {
                            neopixel_data[index].state = N_FLASH_ON;
                            neopixel_data[index].current_colour = neopixel_data[index].on_colour;
                            neopixel_data[index].flash_on_counter = neopixel_data[index].flash_on_time;
                        }   
                    }else {  // must bw N_FLASH_ON state
                        neopixel_data[index].flash_on_counter--;
                        if (neopixel_data[index].flash_on_counter <= 0) {
                            neopixel_data[index].state = N_FLASH_OFF;
                            neopixel_data[index].current_colour = neopixel_data[index].off_colour;
                            neopixel_data[index].flash_off_counter = neopixel_data[index].flash_off_time;
                        }   
                    }
                    break;
                default :
                    break;
            }
        }
        end_time = time_us_32();
        update_task_execution_time(TASK_WRITE_NEOPIXELS, start_time, end_time);
    }
}

//==============================================================================
// Archive area
//==============================================================================

        // for (index = 0 ; index < NOS_ROBOKID_LEDS ; index++) {
        //     switch (temp_LED_data[index].state) {
        //         case LED_OFF : {
        //             gpio_put(temp_LED_data[index].pin_number, false);
        //             break;
        //         }
        //         case LED_ON : {
        //             gpio_put(temp_LED_data[index].pin_number, true);
        //             break;
        //         }
        //         case LED_NO_CHANGE : {
        //             break;
        //         }
        //         case LED_FLASH : {
        //             if (temp_LED_data[index].flash_counter == 0) {
        //                 temp_LED_data[index].flash_counter = temp_LED_data[index].flash_time;
        //                 temp_LED_data[index].flash_value = !temp_LED_data[index].flash_value;
        //                 gpio_put(temp_LED_data[index].pin_number, temp_LED_data[index].flash_value);
        //             } else {
        //                 temp_LED_data[index].flash_counter--;
        //             }
        //             break;
        //         }
        //     }
        // }