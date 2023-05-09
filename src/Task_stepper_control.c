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

// struct stepper_data_s {
//     bool        enable;
//     int32_t     current_step_count;
//     int32_t     target_step_count;
//     int32_t     init_step_count;
//     int32_t     current_step_delay;
//     int32_t     coast_step_delay;
//     bool        flip_direction;
// };

struct stepper_data_s     stepper_data[NOS_STEPPERS] = {
};

error_codes_te calibrate_stepper(void);
void A4988_interface_init(void);
void do_step(uint32_t stepper_id);

struct repeating_timer timer;

//==============================================================================
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
// Task code

void Task_stepper_control(void *p) {

    A4988_interface_init();

    add_repeating_timer_us(1000, repeating_timer_callback, NULL, &timer);
    FOREVER {
        vTaskDelay(10000);    // all the work is done in the callback routine
    }
}

//==============================================================================
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
            if (gpio_get(LIMIT_SWITCH_1 == 1)) {
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
    gpio_init(A4988_STEP);
    gpio_set_dir(A4988_STEP, GPIO_OUT);
    gpio_pull_down(A4988_STEP); 

    gpio_init(A4988_DIRECTION);
    gpio_set_dir(A4988_DIRECTION, GPIO_OUT);
    gpio_pull_down(A4988_DIRECTION); 

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
    gpio_put(A4988_STEP, 1);
    busy_wait_at_least_cycles(250);  // 2uS with a 125MHz clock
    gpio_put(A4988_STEP, 0);
    vTaskDelay(20000);
}