/**
 * @file Task_stepper.c
 * @author Jim Herd
 * @brief manage stepper motors
 */
#include <stdio.h>
#include <stdlib.h>

#include "system.h"
#include "externs.h"

#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "FreeRTOS.h"

//==============================================================================
// Global data
//==============================================================================

struct stepper_data_s     stepper_data[NOS_STEPPERS] = {
    {GP10, GP11, CLOCKWISE, false, 160, 330, NO_PROFILE, 0, 0, M_UNCALIBRATED},
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
struct stepper_data_s  *sm_ptr;

    START_PULSE; 
    for (uint32_t i=0; i<NOS_STEPPERS; i++) {
        sm_ptr = &stepper_data[i];
        switch (sm_ptr->state) {
            case M_DORMANT:
                break;    // do nothing
            case M_INIT:       // run once at the begining of a sm_profile move
                while (sequences[sm_ptr->sm_profile].cmds[sm_ptr->cmd_index].sm_profile_state  == SM_SKIP) {
                    sm_ptr->cmd_index++;
                }
                if (sequences[sm_ptr->sm_profile].cmds[sm_ptr->cmd_index].sm_profile_state  == SM_END) {
                    sm_ptr->state = DORMANT;
                    break;    
                }
                if (sm_ptr->state == SM_COAST) {
                    sm_ptr->current_step_count = sm_ptr->coast_step_count;
                } else {
                    sm_ptr->current_step_count = sequences[sm_ptr->sm_profile].nos_sm_cmds;
                }
                sm_ptr->current_step_delay_count = sequences[sm_ptr->sm_profile].cmds[0].sm_delay;
                gpio_put(sm_ptr->direction_pin, sm_ptr->direction);
                do_step(i);
                sm_ptr->current_step_count++;
                sm_ptr->state = M_RUNNING;  // update state
                break;

            case M_RUNNING :
            // Check if active delay is complete
                if (sm_ptr->current_step_delay_count != 0) {
                    sm_ptr->current_step_delay_count--;    
                    break;   // delay time incomplete so wait for next clock interrupt
                }
            // implement next command
            // jump over any SKIP (NOOP) commands
                while (sequences[sm_ptr->sm_profile].cmds[sm_ptr->cmd_index].sm_profile_state  == SM_SKIP) {
                    sm_ptr->cmd_index++;
                }
            // check for end of profile execution
                if (sequences[sm_ptr->sm_profile].cmds[sm_ptr->cmd_index].sm_profile_state  == SM_END) {
                    sm_ptr->state = DORMANT;   // stepper motor move complete
                    break;    
                }
            // implement new command
                if (sequences[sm_ptr->sm_profile].cmds[sm_ptr->cmd_index].sm_profile_state == SM_COAST) {
                    sm_ptr->current_step_count = sm_ptr->coast_step_count;
                } else {
                    sm_ptr->current_step_count = sequences[sm_ptr->sm_profile].nos_sm_cmds;
                }
                sm_ptr->current_step_delay_count = sequences[sm_ptr->sm_profile].cmds[0].sm_delay;
                do_step(i);
                sm_ptr->current_step_count++;
                break;
            case M_UNCALIBRATED :
                sm_ptr->error = MOVE_ON_UNCALIBRATED_MOTOR;
                break;
            case M_FAULT :
                sm_ptr->error = EXISTING_FAULT_WITH_MOTOR;
                break;
        }
    }
    STOP_PULSE;
    return true;
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
 *          one of the endstop to find a zero position (kown as the origin).
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
            vTaskDelay(10);
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
/**
 * @brief generate a short single step pulse 
 * 
 * @param stepper_id   index of selected stepper motor move sm_profile
 */
void inline do_step(uint32_t stepper_id)
{
    gpio_put(stepper_data[stepper_id].step_pin, ON);
    busy_wait_us(10); 
    gpio_put(stepper_data[stepper_id].step_pin, OFF);
}

//==============================================================================
// Archive : delete after system tests
//
//  struct sm_profile_s  sequences[NOS_PROFILES] = {
//     {7, {{ACCEL,12,1},{ACCEL,9,1},{ACCEL,6,1,},    // fast speed
//      {COAST,3,-1},
//      {DECEL,6,1},{DECEL<9,1},{DECEL,12,1}}},
//  };