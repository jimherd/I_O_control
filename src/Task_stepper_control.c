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
 //   {200, 5, 2, 0, GP17, GP16, GP18, GP19, CLOCKWISE, false, 160, false, 200, -30, +30, 0,0, OK, STATE_SM_DORMANT,0,0,0,0,0,0}
    {200, 1, 2, 0, GP17, GP16, GP18, GP19, CLOCKWISE, false, 100, false, 200, -30, +30, 0,0, OK, STATE_SM_DORMANT,0,0,0,0,0,0}
};

//==============================================================================
// Function templates
//==============================================================================

error_codes_te calibrate_stepper(uint32_t stepper_no);
void TMC2208_interface_init(void);
void do_step(uint32_t stepper_id);
void set_SM_direction(uint32_t stepper_id, sm_direction direction);
void  init_stepper_motor_data(void);

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
 *  2.  Mealy state machine to drive stepper motors
 *      
 */
bool repeating_timer_callback(struct repeating_timer *t) 
{
struct stepper_data_s  *sm_ptr;
error_codes_te  status;

    // START_PULSE; 
    for (uint32_t i=0; i<NOS_STEPPERS; i++) {
        sm_ptr = &stepper_data[i];
        switch (sm_ptr->state) {
            case STATE_SM_DORMANT:
                break;    // do nothing
            case STATE_SM_SYNC :
                break;    // hold until run sync command is initiated
            case STATE_SM_INIT:       // run once at the begining of a sm_profile move
                if (sm_ptr->calibrated == false) {
                    sm_ptr->error = MOVE_ON_UNCALIBRATED_MOTOR;
                    break;
                }
                while (sequences[sm_ptr->sm_profile].cmds[sm_ptr->cmd_index].sm_command_type  == SM_SKIP) {
                    sm_ptr->cmd_index++;
                }
                if (sequences[sm_ptr->sm_profile].cmds[sm_ptr->cmd_index].sm_command_type  == SM_END) {
                    sm_ptr->state = STATE_SM_DORMANT;
                    break;    
                }
                if (sm_ptr->state == SM_COAST) {
                    sm_ptr->cmd_step_cnt = sm_ptr->coast_step_count;
                } else {
                    sm_ptr->cmd_step_cnt = sequences[sm_ptr->sm_profile].cmds[sm_ptr->cmd_index].sm_cmd_step_cnt;
                }
                set_SM_direction(i, sm_ptr->direction);
                sm_ptr->state = STATE_SM_RUNNING;  // update state
                break;

            case STATE_SM_RUNNING :
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
                        sm_ptr->state = STATE_SM_FAULT;   // a limit switch has been activated
                        sm_ptr->error = LIMIT_SWITCH_ERROR;
                        break;
                    }
                    sm_ptr->current_step_delay_count = sequences[sm_ptr->sm_profile].cmds[sm_ptr->cmd_index].sm_delay;
                    do_step(i);
                    if (sm_ptr->direction == CLOCKWISE) {
                        sm_ptr->current_step_count++;
                    } else {
                        sm_ptr->current_step_count--;
                    }
                    sm_ptr->cmd_step_cnt--; 
                    break;
                }
        // Move onto next command
            // jump over any SKIP (NOOP) commands
                sm_ptr->cmd_index++;    // point to next command
                while (sequences[sm_ptr->sm_profile].cmds[sm_ptr->cmd_index].sm_command_type  == SM_SKIP) {
                    sm_ptr->cmd_index++;
                }
            // check for end of profile execution
                if (sequences[sm_ptr->sm_profile].cmds[sm_ptr->cmd_index].sm_command_type  == SM_END) {
                    sm_ptr->state = STATE_SM_DORMANT;   // stepper motor move complete
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
                    sm_ptr->state = STATE_SM_FAULT;   // a limit switch has been activated
                    sm_ptr->error = LIMIT_SWITCH_ERROR;
                    break;
                }
                sm_ptr->current_step_delay_count = sequences[sm_ptr->sm_profile].cmds[sm_ptr->cmd_index].sm_delay;
                do_step(i);
                if (sm_ptr->direction == CLOCKWISE) {
                    sm_ptr->current_step_count++;   
                } else {
                    sm_ptr->current_step_count--;
                }
                sm_ptr->cmd_step_cnt--; 
                break;
            case STATE_SM_UNCALIBRATED :
                sm_ptr->temp_count = MAX_STEPS;
                set_SM_direction(i, CLOCKWISE);
                sm_ptr->state = STATE_SM_CALIB_S0;
                break;
            case STATE_SM_CALIB_S0 : // ensure motor is not triggering LEFT limit switch
                if (gpio_get(sm_ptr->L_limit_pin) == ASSERTED_LOW) {
                    do_step(i); // and stay in this state until clear of LEFT limit switch
                    sm_ptr->temp_count--;
                    if (sm_ptr->temp_count <= 0) {
                        sm_ptr->error = STEPPER_CALIBRATE_FAIL;
                        sm_ptr->state = STATE_SM_DORMANT;
                        break;
                    }
                } else {
                    set_SM_direction(i, ANTI_CLOCKWISE);
                    sm_ptr->state = STATE_SM_CALIB_S1;
                }
                break;
    // states S1,S2,S3 to move to LEFT limit
            case STATE_SM_CALIB_S1 :
                do_step(i);
                if (CALIBRATE_SPEED_DELAY != 0) {
                    sm_ptr->current_step_delay_count = CALIBRATE_SPEED_DELAY;
                    sm_ptr->state = STATE_SM_CALIB_S2;
                } else {
                    sm_ptr->state = STATE_SM_CALIB_S3;
                }
                break;
            case STATE_SM_CALIB_S2 :   // delay until next pulse
                sm_ptr->current_step_delay_count--;
                if (sm_ptr->current_step_delay_count == 0) {
                    sm_ptr->state = STATE_SM_CALIB_S3;  // delay complete
                } 
                break;
            case STATE_SM_CALIB_S3 :
                if (gpio_get(sm_ptr->L_limit_pin) == ASSERTED_LOW) {
                    sm_ptr->current_step_count = 0;
                    sm_ptr->temp_count = MAX_STEPS;
                    set_SM_direction(i, CLOCKWISE);
                    sm_ptr->state = STATE_SM_CALIB_S4;
                } else {
                    sm_ptr->state = STATE_SM_CALIB_S1;
                }
                break;
    // states S4,S5,S6 to move to RIGHT limit
            case STATE_SM_CALIB_S4 :
                do_step(i);
                sm_ptr->current_step_count++;
                sm_ptr->temp_count--;
                if (sm_ptr->temp_count <= 0) {
                    sm_ptr->error = STEPPER_CALIBRATE_FAIL;
                    sm_ptr->state = STATE_SM_DORMANT;
                    break;
                }
                if (CALIBRATE_SPEED_DELAY != 0) {
                    sm_ptr->current_step_delay_count = CALIBRATE_SPEED_DELAY;
                    sm_ptr->state = STATE_SM_CALIB_S5;
                } else {
                    sm_ptr->state = STATE_SM_CALIB_S6;
                }
                break;
            case STATE_SM_CALIB_S5 :
                sm_ptr->current_step_delay_count--;
                if (sm_ptr->current_step_delay_count == 0) {
                    sm_ptr->state = STATE_SM_CALIB_S6;  // delay complete
                } 
                break;
            case STATE_SM_CALIB_S6 :
                if (gpio_get(sm_ptr->R_limit_pin) == ASSERTED_LOW) {
                    sm_ptr->state = STATE_SM_CALIB_S7;
                } else {
                    sm_ptr->state = STATE_SM_CALIB_S4;
                }
                break;
    // Prepare to move to initial position
            case STATE_SM_CALIB_S7 :
                sm_ptr->max_step_count = sm_ptr->current_step_count;
                sm_ptr->calibrated = true;
                set_SM_direction(i, ANTI_CLOCKWISE);
                sm_ptr->temp_count = sm_ptr->max_step_count - sm_ptr->init_step_position;
                sm_ptr->state = STATE_SM_CALIB_S8;
                break;
    // states S8,S9,S10 to move to initial position             
            case STATE_SM_CALIB_S8 :
                do_step(i);
                sm_ptr->current_step_count--;
                sm_ptr->temp_count--;
                if (sm_ptr->temp_count <= 0) {
                    sm_ptr->error = OK;
                    sm_ptr->state = STATE_SM_DORMANT;
                    break;
                }
                if (CALIBRATE_SPEED_DELAY != 0) {
                    sm_ptr->current_step_delay_count = CALIBRATE_SPEED_DELAY;
                    sm_ptr->state = STATE_SM_CALIB_S9;
                } else {
                    sm_ptr->state = STATE_SM_CALIB_S10;
                }
                break;
            case STATE_SM_CALIB_S9 :
                sm_ptr->current_step_delay_count--;
                if (sm_ptr->current_step_delay_count == 0) {
                    sm_ptr->state = STATE_SM_CALIB_S10;  // delay complete
                } 
                break;
            case STATE_SM_CALIB_S10 :
                if (sm_ptr->temp_count == 0) {
                    sm_ptr->state = STATE_SM_CALIB_S11;
                }  else {
                    sm_ptr->state = STATE_SM_CALIB_S8;
                }
                break;
    // calibrate complete
            case STATE_SM_CALIB_S11 :
                sm_ptr->error = OK;
                sm_ptr->state = STATE_SM_DORMANT;
                break;
                
            case STATE_SM_FAULT :
                break;
            default :
                sm_ptr->error = UNKNOWN_STEPPER_MOTOR_STATE;
                break;
        }
    }
    // STOP_PULSE;
    return true;
}

//==============================================================================
//==============================================================================
// Task code

void Task_stepper_control(void *p) 
{
    struct stepper_data_s  *sm_ptr;

    init_stepper_motor_data();
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
 *          Routine uses simple pulse code rather than the interrupt driven
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
        set_SM_direction(stepper_no, ANTI_CLOCKWISE);
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
        set_SM_direction(stepper_no, CLOCKWISE);
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
    set_SM_direction(stepper_no, ANTI_CLOCKWISE);
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
    set_SM_direction(stepper_no, CLOCKWISE);
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
    set_SM_direction(stepper_no, ANTI_CLOCKWISE);
    for (j = 0; j < (stepper_data[stepper_no].max_step_count >> 1); j++) {
        do_step(stepper_no);
        stepper_data[stepper_no].current_step_count -= 1;
        vTaskDelay(10);
    }
    stepper_data[stepper_no].calibrated = true;
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

    }
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
    busy_wait_us(1); 
    gpio_put(stepper_data[stepper_id].step_pin, OFF);
}

/**
 * @brief Set the direction object
 * 
 * @param   stepper_id      active stepper motor
 * @param   direction       CLOCKWISE or ANTI_CLOCKWISE
 */
void inline set_SM_direction(uint32_t stepper_id, sm_direction direction)
{
    if (stepper_data[stepper_id].flip_direction == true) {
        if (direction == CLOCKWISE) {
            direction = ANTI_CLOCKWISE;
        } else {
            direction = CLOCKWISE;
        }
    }
    gpio_put(stepper_data[stepper_id].direction_pin, direction);
}

/**
 * @brief set some initial runtime parameters
 * 
 * 1. set temp value for max_step_count (refined by calibration)
 * 2. calculate steps per degree
 * 
 */
void  init_stepper_motor_data(void) {

struct stepper_data_s  *sm_ptr;
error_codes_te  status;

    for (uint32_t i=0; i<NOS_STEPPERS; i++) {
        sm_ptr = &stepper_data[i];
        sm_ptr->max_step_count = sm_ptr->steps_per_rev + sm_ptr->gearbox_ratio;
        sm_ptr->steps_per_degree 
            = (float)(sm_ptr->steps_per_rev * sm_ptr->gearbox_ratio * sm_ptr->microstep_value) / 360.0;
    }
}

