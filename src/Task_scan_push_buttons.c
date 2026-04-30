/**
 * @file    Task_scan_push_buttons.c
 * @author  Jim Herd
 * @brief   Read and debounce user I/O push buttons
 */

#include "system.h"
#include "Pico_IO.h"
#include "sys_routines.h"
#include "externs.h"

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/regs/addressmap.h"

#include "FreeRTOS.h"
#include "task.h"

struct switch_data_s switch_data;

//==============================================================================
// Main task routine
//==============================================================================
//

void Task_scan_push_buttons(void *p) 
{
TickType_t  xLastWakeTime;
BaseType_t  xWasDelayed;
uint8_t     index;
uint32_t    start_time, end_time;
uint32_t    sample_index;

volatile struct switch_data_s switch_data;

//==============================================================================
// Task code
//==============================================================================
    sample_index = 0;
    for(index=0; index < NOS_SWITCH_SAMPLES; index++) { 
        switch_data.raw_switch_values[index] = 0; 
    }
    xLastWakeTime = xTaskGetTickCount ();
    FOREVER {
        xWasDelayed = xTaskDelayUntil( &xLastWakeTime, TASK_SCAN_PUSH_BUTTONS_FREQUENCY_TICK_COUNT );
        start_time = time_us_32();

        switch_data.raw_switch_values[sample_index++] = ~(gpio_get_all() & (0b1111 << GP10));
        switch_data.debounced_state = (0b1111 << GP10);
        for(index=0; index < NOS_SWITCH_SAMPLES; index++) { 
            switch_data.debounced_state = switch_data.debounced_state & switch_data.raw_switch_values[index]; 
        }
        if(sample_index >= NOS_SWITCH_SAMPLES) {
            sample_index = 0; 
        }

    // extract switch bits
  
        switch_data.switch_value[SWITCH_A] = ((switch_data.debounced_state & (1<<GP10)) >> GP10 )  & 0b1;
        switch_data.switch_value[SWITCH_B] = ((switch_data.debounced_state & (1<<GP11)) >> GP11 )  & 0b1;
        switch_data.switch_value[SWITCH_C] = ((switch_data.debounced_state & (1<<GP12)) >> GP12 )  & 0b1;
        switch_data.switch_value[SWITCH_D] = ((switch_data.debounced_state & (1<<GP13)) >> GP13 )  & 0b1;
        switch_data.switches_ABCD = switch_data.debounced_state & (0b1111 << GP10);

        end_time = time_us_32();
        update_task_execution_time(TASK_SCAN_PUSH_BUTTONS, start_time, end_time);
    }
}  
