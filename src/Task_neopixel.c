/**
 * @file    Task_RW_sensors.c
 * @author  Jim Herd
 * @brief   read Robokid sensors and set neopixel LEDs
 */

#include <string.h>
#include <assert.h>

#include "system.h"
#include "Pico_IO.h"
#include "sys_routines.h"
#include "externs.h"
//#include "common.h"
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

typedef struct seq_buffer_s {
    colours_et   colour;
    uint8_t     intensity;   // 0->100%
} seq_buffer_s;

//==============================================================================
// function prototypes for local routines
//==============================================================================

//==============================================================================
// Local globals
//==============================================================================

seq_buffer_s seq_buffer_1[NOS_NEOPIXELS];


//==============================================================================
// temp locals
//==============================================================================

//==============================================================================
// Main task routine
//==============================================================================
//
// 1. read push buttons and debounce
// 2. read line sensors
// 3. read analogue sensors
// 3. update central data store

void Task_write_neopixels(void *p) 
{
TickType_t  xLastWakeTime;
BaseType_t  xWasDelayed;
uint8_t     index;
uint32_t    start_time, end_time;

    uint offset = pio_add_program(NEOPIXEL_PIO_UNIT, &neopixel_program);
    neopixel_program_init(NEOPIXEL_PIO_UNIT, 
                        NEOPIXEL_STATE_MACHINE, 
                        offset, 
                        NEOPIXEL_DOUT_PIN, 
                        NEOPIXEL_DATA_RATE,
                        NEOPIXEL_BITS_PER_UNIT 
                        );
//
// Some test data
//
        put_pixel(urgb_u32(0x1f, 0, 0));       // Red
        put_pixel(urgb_u32(0, 0x1f, 0));       // Green
        put_pixel(urgb_u32(0, 0, 0x1f));       // Blue
        put_pixel(urgb_u32(0x10, 0x10, 0x10)); // 

//==============================================================================
// Task code
//==============================================================================
    xLastWakeTime = xTaskGetTickCount ();
    FOREVER {
        xWasDelayed = xTaskDelayUntil( &xLastWakeTime, TASK_NEOPIXELS_FREQUENCY_TICK_COUNT );
        start_time = time_us_32();

// Process LED data

        put_pixel(urgb_u32(0x1f, 0, 0));  // Red
        put_pixel(urgb_u32(0, 0x1f, 0));
        put_pixel(urgb_u32(0, 0, 0x1f));
        put_pixel(urgb_u32(0x10, 0x10, 0x10));

        for (index = 0 ; index < NOS_NEOPIXEL_LEDS ; index++) {
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