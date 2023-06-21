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
    {200, 5, 2, 0, GP17, GP16, GP18, GP19, CLOCKWISE, false, 160, false, 330, 0,0, OK, M_DORMANT,0,0,0,0,0,0}
};

//==============================================================================
// Function templates
//==============================================================================

error_codes_te calibrate_stepper(uint32_t stepper_no);
void TMC2208_interface_init(void);
void do_step(uint32_t stepper_id);

struct repeating_timer timer;

//==============================================================================
//==============================================================================
// Interrupt routines
/**
 * @brief regular timer interrupt to manage stepper motors
 * 
 * @param t         Pointer to timer structure
 * @return true 
 * @return false 
 * 
 * @notes
 *      
 *  1.  Set error if attempt to move an uncalibrated motor.
 *  2.  Calibration activity causes interrupt driven stepper motor activity
 *      to be suspended. (May be changed in future)
 *      
 */
bool repeating_timer_callback(struct repeating_timer *t) 
{
struct stepper_data_s  *sm_ptr;
error_codes_te  status;

    START_PULSE; 
    for (uint32_t i=0; i<NOS_STEPPERS; i++) {
        sm_ptr = &stepper_data[i];
        switch (sm_ptr->state) {
            case M_DORMANT:
                break;    // do nothing
            case M_SYNC :
                break;    // hold until run sync command is initiated
            case M_INIT:       // run once at the begining of a sm_profile move
                if (sm_ptr->calibrated == false) {
                    sm_ptr->error = MOVE_ON_UNCALIBRATED_MOTOR;
                    break;
                }
                while (sequences[sm_ptr->sm_profile].cmds[sm_ptr->cmd_index].sm_command_type  == SM_SKIP) {
                    sm_ptr->cmd_index++;
                }
                if (sequences[sm_ptr->sm_profile].cmds[sm_ptr->cmd_index].sm_command_type  == SM_END) {
                    sm_ptr->state = DORMANT;
                    break;    
                }
                if (sm_ptr->state == SM_COAST) {
                    sm_ptr->cmd_step_cnt = sm_ptr->coast_step_count;
                } else {
                    sm_ptr->cmd_step_cnt = sequences[sm_ptr->sm_profile].cmds[sm_ptr->cmd_index].sm_cmd_step_cnt;
                }
                if (sm_ptr->direction == CLOCKWISE) {
                    gpio_put(sm_ptr->direction_pin, 1);
                } else {
                    gpio_put(sm_ptr->direction_pin, 0);
                }
                sm_ptr->state = M_RUNNING;  // update state
                break;

            case M_RUNNING :
                if (sm_ptr->calibrated == false) {
                    sm_ptr->error = MOVE_ON_UNCALIBRATED_MOTOR;
                    break;
                }
        // Check if active delay is complete
                if (sm_ptr->current_step_delay_count != 0) {
                    sm_ptr->current_step_delay_count--;  
                    break;   // delay time incomplete so wait for next timer interrupt
                }
        // check if more steps at this speed are required
                if (sm_ptr->cmd_step_cnt != 0) {
                // check for unexpected trigerring of a limit switch
                    if (gpio_get(sm_ptr->R_limit_pin) == ASSERTED_LOW || gpio_get(sm_ptr->R_limit_pin) == ASSERTED_LOW) {
                        sm_ptr->state = M_FAULT;   // a limit switch has been activated
                        sm_ptr->error = LIMIT_SWITCH_ERROR;
                        break;
                    }
                    sm_ptr->current_step_delay_count = sequences[sm_ptr->sm_profile].cmds[sm_ptr->cmd_index].sm_delay;
                    do_step(i);
                    sm_ptr->current_step_count += sm_ptr->direction;
                    sm_ptr->cmd_step_cnt--; 
                    break;
                }
        // Move onto next command
            // jump over any SKIP (NOOP) commands
                sm_ptr->cmd_index++;    // point tonext command
                while (sequences[sm_ptr->sm_profile].cmds[sm_ptr->cmd_index].sm_command_type  == SM_SKIP) {
                    sm_ptr->cmd_index++;
                }
            // check for end of profile execution
                if (sequences[sm_ptr->sm_profile].cmds[sm_ptr->cmd_index].sm_command_type  == SM_END) {
                    sm_ptr->state = DORMANT;   // stepper motor move complete
                    break;    
                }
            // implement new command
                if (sequences[sm_ptr->sm_profile].cmds[sm_ptr->cmd_index].sm_command_type == SM_COAST) {
                    sm_ptr->cmd_step_cnt = sm_ptr->coast_step_count;
                } else {
                    sm_ptr->cmd_step_cnt = sequences[sm_ptr->sm_profile].cmds[sm_ptr->cmd_index].sm_cmd_step_cnt;
                }
                // check for unexpected trigerring of a limit switch
                if (gpio_get(sm_ptr->R_limit_pin) == ASSERTED_LOW || gpio_get(sm_ptr->R_limit_pin) == ASSERTED_LOW) {
                    sm_ptr->state = M_FAULT;   // a limit switch has been activated
                    sm_ptr->error = LIMIT_SWITCH_ERROR;
                    break;
                }
                sm_ptr->current_step_delay_count = sequences[sm_ptr->sm_profile].cmds[sm_ptr->cmd_index].sm_delay;
                do_step(i);
                sm_ptr->current_step_count += sm_ptr->direction;
                sm_ptr->cmd_step_cnt--; 
                break;
            case M_UNCALIBRATED :
                if (sm_ptr->state == M_UNCALIBRATED) {
                    cancel_repeating_timer(&timer);
                    status = calibrate_stepper(i);
                    add_repeating_timer_us(1000, repeating_timer_callback, NULL, &timer);
                }
                if (status == OK) {
                    sm_ptr->calibrated = true;
                    sm_ptr->state = M_DORMANT;
                }
                // error set by calibrate routine
                break;
            case M_FAULT :
                sm_ptr->error = EXISTING_FAULT_WITH_MOTOR;
                break;
            default :
                sm_ptr->error = UNKNOWN_STEPPER_MOTOR_STATE;
                break;
        }
    }
    STOP_PULSE;
    return true;
}

//==============================================================================
//==============================================================================
// Task code

void Task_stepper_control(void *p) 
{
    struct stepper_data_s  *sm_ptr;

    TMC2208_interface_init();

    for (uint32_t i=0; i<NOS_STEPPERS; i++) {
        calibrate_stepper(i);
    }

    add_repeating_timer_us(1000, repeating_timer_callback, NULL, &timer);
    FOREVER {
        vTaskDelay(1000);    // all the work is done in the callback routine
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
 * 
 *         
 */
error_codes_te calibrate_stepper(uint32_t stepper_no)
{
error_codes_te  status;
uint32_t    i, j;

status = STEPPER_CALIBRATE_FAIL;

    // ensure stepper is not triggering either limit switches. Move if necessary.
    if (stepper_data[stepper_no].L_limit_pin == ASSERTED_LOW) {
            gpio_put(stepper_data[stepper_no].direction_pin, ANTI_CLOCKWISE);
            for (j = 0; j < MAX_STEPS; j++) {
                do_step(i);
                vTaskDelay(10);
                if (gpio_get(stepper_data[stepper_no].L_limit_pin) != ASSERTED_LOW) {
                    stepper_data[stepper_no].current_step_count = 0;
                    status = OK;  // found origin point
                    break;
                }
                if (status == STEPPER_CALIBRATE_FAIL) {
                    stepper_data[stepper_no].error = STEPPER_CALIBRATE_FAIL;
                    return status;
                }
            }
    }
    if (stepper_data[stepper_no].R_limit_pin == ASSERTED_LOW) {
            gpio_put(stepper_data[stepper_no].direction_pin, CLOCKWISE);
            for (j = 0; j < MAX_STEPS; j++) {
                do_step(stepper_no);
                vTaskDelay(10);
                if (gpio_get(stepper_data[stepper_no].L_limit_pin) != ASSERTED_LOW) {
                    stepper_data[stepper_no].current_step_count = 0;
                    status = OK;  // found origin point
                    break;
                }
                if (status == STEPPER_CALIBRATE_FAIL) {
                    stepper_data[stepper_no].error = STEPPER_CALIBRATE_FAIL;
                    return status;
                }
            }
    }
    // move to origin point limit switch
    gpio_put(stepper_data[stepper_no].direction_pin, ANTI_CLOCKWISE);
    for (j = 0; j < MAX_STEPS; j++) {
            do_step(stepper_no);
            vTaskDelay(10);
            if (gpio_get(stepper_data[stepper_no].L_limit_pin) == ASSERTED_LOW) {
                stepper_data[stepper_no].current_step_count = 0;
                status = OK;  // found origin point
                break;
            }
    }
    if (status == STEPPER_CALIBRATE_FAIL) {
            stepper_data[stepper_no].error = STEPPER_CALIBRATE_FAIL;
            return status;
    }
    // move to maximum point limit switch and record step count
    status = STEPPER_CALIBRATE_FAIL;
    gpio_put(stepper_data[stepper_no].direction_pin, CLOCKWISE);
    for (j = 0; j < MAX_STEPS; j++) {
            do_step(stepper_no);
            stepper_data[stepper_no].current_step_count += 1;
            vTaskDelay(10);
            if (gpio_get(stepper_data[stepper_no].R_limit_pin) == ASSERTED_LOW) {
                stepper_data[stepper_no].max_step_count = j;
                status = OK;  // found end limit
                break;
            }
    }
    if (status == STEPPER_CALIBRATE_FAIL) {
            stepper_data[stepper_no].error = STEPPER_CALIBRATE_FAIL;
            return status;
    }
    // move to mid position
    gpio_put(stepper_data[stepper_no].direction_pin, ANTI_CLOCKWISE);
    for (j = 0; j < (stepper_data[stepper_no].max_step_count >> 1); j++) {
            do_step(stepper_no);
            stepper_data[stepper_no].current_step_count -= 1;
            vTaskDelay(10);
    }
    return status;
}

//==============================================================================
void TMC2208_interface_init(void)
{
    for (uint32_t i=0 ; i < NOS_STEPPERS ; i++) {

        gpio_init(stepper_data[i].step_pin);
        gpio_set_dir(stepper_data[i].step_pin, GPIO_OUT);
        gpio_pull_down(stepper_data[i].step_pin); 

        gpio_init(stepper_data[i].direction_pin);
        gpio_set_dir(stepper_data[i].direction_pin, GPIO_OUT);
        gpio_pull_down(stepper_data[i].direction_pin);
        
        gpio_init(stepper_data[i].L_limit_pin);
        gpio_set_dir(stepper_data[i].L_limit_pin, GPIO_IN);
        gpio_pull_up(stepper_data[i].L_limit_pin);

        gpio_init(stepper_data[i].R_limit_pin);
        gpio_set_dir(stepper_data[i].R_limit_pin, GPIO_IN);
        gpio_pull_up(stepper_data[i].R_limit_pin);

        stepper_data[i].steps_per_degree 
                = (float)(stepper_data[i].steps_per_rev * stepper_data[i].gearbox_ratio * stepper_data[i].microstep_value) / 360.0;
    }
}

//==============================================================================
/**
 * @brief generate a short single step pulse 
 * 
 * @param stepper_id   index of selected stepper motor move sm_profile
 */
//void inline do_step(uint32_t stepper_id)
void inline do_step(uint32_t stepper_id)
{
    gpio_put(stepper_data[stepper_id].step_pin, ON);
    busy_wait_us(1); 
    gpio_put(stepper_data[stepper_id].step_pin, OFF);
}

