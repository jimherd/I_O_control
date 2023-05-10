/**
 * @file Task_stepper.c
 * @author Jim Herd
 * @brief manage stepper motors
 */
#include <stdio.h>
#include <stdlib.h>

#include "system.h"

#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "FreeRTOS.h"

//==============================================================================
// Global data
//==============================================================================

struct stepper_data_s     stepper_data[NOS_STEPPERS] = {
    {false, GP10, GP11, false, 50, 10},
};

error_codes_te calibrate_stepper(void);
void A4988_interface_init(void);
void do_step(uint32_t stepper_id);

struct repeating_timer timer;

//==============================================================================
//==============================================================================
// Interrupt routines
/**
 * @brief regular timer interrupt to manage stepper motors
 * 
 * @param t 
 * @return true 
 * @return false 
 * 
 * @notes
 *      
 */
bool repeating_timer_callback(struct repeating_timer *t) 
{
    return 0;
}

//==============================================================================
//==============================================================================
// Task code

void Task_stepper_control(void *p) {

    A4988_interface_init();

    calibrate_stepper();

    add_repeating_timer_us(1000, repeating_timer_callback, NULL, &timer);
    FOREVER {
        vTaskDelay(10000);    // all the work is done in the callback routine
    }
}

//==============================================================================
//==============================================================================
// Functions
/**
 * @brief   RE-calibrate stepper to compensate for lost steps
 * 
 * @return error_codes_te 
 * 
 * @notes   As stepper motors are openloop devices there is a possibility that
 *          steps can be lost without knowing that this has happened.
 *          The simplest solution is to have a switch at each endstop to specify
 *          two known positions.  Calibration involves moving the stepper to
 *          one of the endstop to find a zero position.
 * 
 *          Routine uses simple pulse code rather then the interrupt driven
 *          software used for normal moves.
 */
error_codes_te calibrate_stepper(void)
{
error_codes_te  status;

    status = STEPPER_CALIBRATE_FAIL;
    for (uint32_t i = 0 ; i < NOS_STEPPERS ; i++) {
        gpio_put(A4988_DIRECTION, ANTI_CLOCKWISE);
        for (uint32_t j=0 ; j < MAX_STEPS ; j++) {  
            do_step(i);
            if (gpio_get(LIMIT_SWITCH_1 == ON)) {
                status = OK;    // found origin point
                break;
            }
        }
    }
    return status;
}

//==============================================================================
void A4988_interface_init(void)
{
    for (uint32_t i=0 ; i < NOS_STEPPERS ; i++) {

        gpio_init(stepper_data[i].step_pin);
        gpio_set_dir(stepper_data[i].step_pin, GPIO_OUT);
        gpio_pull_down(stepper_data[i].step_pin); 

        gpio_init(stepper_data[i].direction_pin);
        gpio_set_dir(stepper_data[i].direction_pin, GPIO_OUT);
        gpio_pull_down(stepper_data[i].direction_pin); 
    }

    gpio_init(LIMIT_SWITCH_1);
    gpio_set_dir(LIMIT_SWITCH_1, GPIO_IN);
    gpio_pull_down(LIMIT_SWITCH_1); 

    gpio_init(LIMIT_SWITCH_2);
    gpio_set_dir(LIMIT_SWITCH_2, GPIO_IN);
    gpio_pull_down(LIMIT_SWITCH_2); 
}

//==============================================================================
void do_step(uint32_t stepper_id)
{
    gpio_put(stepper_data[stepper_id].step_pin, ON);
    busy_wait_us(25); 
    gpio_put(stepper_data[stepper_id].step_pin, OFF);
    vTaskDelay(10);
}