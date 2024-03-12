/**
 * @file Task_run_cmd.c
 * @author Jim Herd
 * @brief Read and execute command strings
 */
#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"

#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "externs.h"
#include "system.h"
#include "uart_IO.h"
#include "string_IO.h"
#include "sys_routines.h"
#include "PCA9685.h"
#include "tokenizer.h"
#include  "Pico_IO.h"

//***************************************************************************
// Function prototypes

int32_t parse_command (void);
int32_t convert_tokens(void);
error_codes_te check_command(int32_t cmd_token);

//***************************************************************************
// Global command and parsed data

char        command[MAX_COMMAND_LENGTH];
uint32_t    character_count;
uint32_t    argc, arg_pt[MAX_ARGC], arg_type[MAX_ARGC];
int32_t     int_parameters[MAX_ARGC];
float       float_parameters[MAX_ARGC];

//***************************************************************************
// Get, parse, and execute UART received command

void Task_run_cmd(void *p) 
{
struct servo_data_s     *servo_pt;
error_codes_te          status;
static int32_t          token;
bool                    reply_done;
int32_t                 sm_number;
int32_t                 rel_nos_steps, abs_nos_steps, move_count, move_angle;
uint32_t                current_form;

    status = OK;
    FOREVER {
        character_count = uart_readline(command);
        status = parse_command();
        if (status != OK) {
            print_error(UNDEFINED_PORT, status);
            continue;
        }
        status = convert_tokens();
        if (status != OK) {
            print_error(UNDEFINED_PORT, status);
            continue;
        }
        token = string_to_token(commands, &command[arg_pt[0]]);
        status = check_command(token);
        if (status != OK) {
            print_error(int_parameters[1], status);
            continue;
        }

        reply_done = false;
        status = OK;
        switch (token) {
            case TOKENIZER_SERVO: 
                switch (int_parameters[SERVO_CMD_INDEX]) {
                    case ABS_MOVE: 
                        status = set_servo_move( int_parameters[SERVO_NUMBER_INDEX], MOVE, int_parameters[SERVO_ANGLE_INDEX], false);
                        break;
                    case ABS_MOVE_SYNC: 
                        status = set_servo_move( int_parameters[SERVO_NUMBER_INDEX], MOVE, int_parameters[SERVO_ANGLE_INDEX], true);
                        break;
                    case SPEED_MOVE: 
                        status = set_servo_speed_move(int_parameters[SERVO_NUMBER_INDEX], TIMED_MOVE, int_parameters[SERVO_ANGLE_INDEX], int_parameters[SERVO_SPEED_INDEX], false);
                        break;
                    case SPEED_MOVE_SYNC: 
                        status = set_servo_speed_move(int_parameters[SERVO_NUMBER_INDEX], TIMED_MOVE, int_parameters[SERVO_ANGLE_INDEX], int_parameters[SERVO_SPEED_INDEX], true);
                        break;
                    case RUN_SYNC_MOVES: 
                        status = set_servo_move(int_parameters[SERVO_CMD_INDEX], int_parameters[SERVO_NUMBER_INDEX], int_parameters[SERVO_ANGLE_INDEX], false);
                        break;
                    case STOP:
                        status = set_servo_move(int_parameters[SERVO_CMD_INDEX], int_parameters[SERVO_NUMBER_INDEX], int_parameters[SERVO_ANGLE_INDEX], false);  
                        break;
                    case STOP_ALL: 
                        status = set_servo_move(int_parameters[SERVO_CMD_INDEX], int_parameters[SERVO_NUMBER_INDEX], int_parameters[SERVO_ANGLE_INDEX], false);
                        break;
                    case ENABLE :
                        status = set_servo_state(int_parameters[SERVO_NUMBER_INDEX], DISABLED, int_parameters[SERVO_ANGLE_INDEX]);
                        break;
                    default:
                        status = BAD_SERVO_COMMAND;
                        break;
                }  // end of inner switch
                print_string("%d %d\n", int_parameters[1], status);
                reply_done = true;
                break;

            case TOKENIZER_STEPPER: 
                if (stepper_data[int_parameters[STEP_MOTOR_NO_INDEX]].state != STATE_SM_DORMANT) {
                    status = STEPPER_BUSY;
                }
                if (stepper_data[int_parameters[STEP_MOTOR_NO_INDEX]].error != OK) {  // ensure motor is not in an error state
                    status = stepper_data[int_parameters[3]].error;
                    break;
                }
                sm_number = int_parameters[STEP_MOTOR_NO_INDEX];
                switch (int_parameters[STEP_MOTOR_CMD_INDEX]) { 

                    case SM_REL_MOVE : 
                    case SM_REL_MOVE_SYNC :
                        if (stepper_data[int_parameters[STEP_MOTOR_NO_INDEX]].error != OK) {  // ensure motor is not in an error state
                            status = stepper_data[int_parameters[STEP_MOTOR_NO_INDEX]].error;
                            break;  // existing error => abort move
                        }
                        sm_number = int_parameters[STEP_MOTOR_NO_INDEX];
                        if (abs(int_parameters[STEP_MOTOR_ANGLE_INDEX]) < MIN_STEP_MOVE ) {
                            status = SM_MOVE_TOO_SMALL;
                            break;
                        }

                        rel_nos_steps = (int32_t)(stepper_data[sm_number].steps_per_degree * int_parameters[STEP_MOTOR_ANGLE_INDEX]);
                        move_count = stepper_data[sm_number].current_step_count + rel_nos_steps;
                        if ((move_count < 0) || (move_count > stepper_data[sm_number].max_step_count)) {
                            status = BAD_STEP_VALUE;
                            break;
                        }
                        if (rel_nos_steps < 0) {
                            stepper_data[sm_number].direction = ANTI_CLOCKWISE;
                        } else {
                            stepper_data[sm_number].direction = CLOCKWISE;
                        }
                        if (stepper_data[sm_number].flip_direction == true) {
                            FLIP_BOOLEAN(stepper_data[sm_number].direction);
                        }
                        stepper_data[sm_number].target_step_count = abs(rel_nos_steps);
                        stepper_data[sm_number].sm_profile = 0;
                        stepper_data[sm_number].coast_step_count = (abs(rel_nos_steps) - (sequences[sm_number].nos_sm_cmds - 1));
                        stepper_data[sm_number].cmd_index = 0;
                        if (int_parameters[STEP_MOTOR_CMD_INDEX] == SM_REL_MOVE) {
                            stepper_data[sm_number].state = STATE_SM_INIT;
                        } else {
                            stepper_data[sm_number].state = STATE_SM_SYNC;
                        }
                        break;

                    case SM_ABS_MOVE :
                        if (stepper_data[int_parameters[STEP_MOTOR_NO_INDEX]].error != OK) {  // ensure motor is not in an error state
                            status = stepper_data[int_parameters[STEP_MOTOR_NO_INDEX]].error;
                            break;   // existing error => abort move
                        }
                        sm_number = int_parameters[STEP_MOTOR_NO_INDEX];
                        move_angle = int_parameters[STEP_MOTOR_ANGLE_INDEX];
                        if ((move_angle < stepper_data[sm_number].soft_left_limit) || (move_angle > stepper_data[sm_number].soft_right_limit)) {
                            status = BAD_STEP_VALUE;
                            break;
                        }
                        
                        abs_nos_steps = (int32_t)(stepper_data[sm_number].steps_per_degree * (move_angle + stepper_data[sm_number].soft_right_limit));
                        move_count = abs(stepper_data[sm_number].current_step_count - abs_nos_steps);
                        if (abs_nos_steps < 0) {
                            stepper_data[sm_number].direction = ANTI_CLOCKWISE;
                        } else {
                            stepper_data[sm_number].direction = CLOCKWISE;
                        }
                        if (stepper_data[sm_number].flip_direction == true) {
                            FLIP_BOOLEAN(stepper_data[sm_number].direction);
                        }
                        stepper_data[sm_number].target_step_count = abs(move_count);
                        stepper_data[sm_number].sm_profile = sm_number;
                        stepper_data[sm_number].coast_step_count = abs(move_count - (sequences[sm_number].nos_sm_cmds - 1));
                        stepper_data[sm_number].cmd_index = 0;
                        if (int_parameters[STEP_MOTOR_NO_INDEX] == SM_ABS_MOVE) {
                            stepper_data[sm_number].state = STATE_SM_INIT;
                        } else {
                            stepper_data[sm_number].state = STATE_SM_SYNC;
                        }
                        break;
                    case SM_CALIBRATE : 
                        stepper_data[sm_number].state = STATE_SM_UNCALIBRATED;
                        break;  // set system to do a calibration on this motor
                    default:
                        status = BAD_STEPPER_COMMAND;
                        break;
                }
                print_string("%d %d\n", int_parameters[1], status);
                reply_done = true;
                break;

            case TOKENIZER_SYNC: 
                for( int32_t i=0; i<NOS_SERVOS; i++) {
                    servo_pt = &servo_data[i];
                    servo_pt->sync = false;
                }
                for (int32_t i=0 ; i <NOS_STEPPERS;i++) {
                    if (stepper_data[i].state == STATE_SM_SYNC) {
                        stepper_data[i].state = STATE_SM_INIT;
                    }
                }
                break;

            case TOKENIZER_SET: 
                break;

            case TOKENIZER_GET:
                switch (int_parameters[2]) {
                    case SYS_INFO:
                        print_string("%d %d %d\n", int_parameters[1], NOS_SERVOS, NOS_STEPPERS);
                        reply_done = true;
                        break;
                    case SERVO_INFO:
                        print_string("%d\n", int_parameters[1]);
                        reply_done = true;
                        break;
                    case STEPPER_INFO:
                        print_string("%d\n", int_parameters[1]);
                        reply_done = true;
                        break;
                    default:
                        break;
                }
                break;

            case TOKENIZER_PING: 
                print_string("%d %d %d\n", int_parameters[1], OK, (int_parameters[2] + 1));
                reply_done = true;
                break;

            case TOKENIZER_DISPLAY:
                switch (int_parameters[DISPLAY_CMD_INDEX]) {
                    case SET_FORM: 
                        if (int_parameters[DISPLAY_FORM_INDEX] > NOS_FORMS) {
                            status = GEN4_uLCD_CMD_BAD_FORM_INDEX;
                            break;
                        }
                        status = gen4_uLCD_WriteObject(GEN4_uLCD_OBJ_FORM, int_parameters[DISPLAY_FORM_INDEX], 0);
                        break;
                    case GET_FORM:
                        break;
                    case SET_CONTRAST:
                        status = gen4_uLCD_WriteContrast(int_parameters[DISPLAY_CONTRAST_INDEX]);
                        break;
                    case READ_BUTTON:
                        current_form = get_active_form();
                        if (button_data[int_parameters[3]].form == current_form) {
                            status = OK;
                        } else {
                            status = GEN4_uLCD_BUTTON_FORM_INACTIVE;
                        }
                        print_string("%d %d %d\n", int_parameters[1], status, button_data[int_parameters[3]].button_value );
                        reply_done = true;
                        break;
                    case WRITE_STRING:
                        current_form = get_active_form();
                        status = gen4_uLCD_WriteString(int_parameters[3], &command[arg_pt[4]]);
                    default:
                        break;
                }
                break;

            case TOKENIZER_TDELAY:
                vTaskDelay(int_parameters[2]);
                break;
                
        }  // end of outer switch
        if (reply_done == false) {
            print_error(int_parameters[1], status);
        }
    }
}

//***************************************************************************
// parse_command : analyse command string and convert into labelled strings
//
// Breaks the command string into a set of token strings that are 
// labelled REAL, INTEGER or STRING.  
//
// Code uses a STATE MACHINE to walkthrough the command string.  Refer
// to associated documentation.  

// defines modes as scan progresses : U=undefined, I=integer, R=real, S=string
//

int32_t parse_command (void) 
{
int32_t     count, mode, status;
uint8_t     character_type;

    argc = 0;
    mode = MODE_U;
    status = OK;
    for (count=0 ; count <= character_count ; count++) {
        character_type = char_type[command[count]];
        switch (character_type) {
            case LETTER :
                switch(mode) {
                    case MODE_U :
                        mode = MODE_S;
                        arg_pt[argc] = count;
                        break;
                    case MODE_I :
                    case MODE_R :
                        status = LETTER_ERROR;
                        break;
                    case MODE_S :
                        break;
                }
                break;

            case QUOTE :
                switch(mode) {
                    case MODE_U :
                        mode = MODE_S;
                        arg_pt[argc] = count + 1;   // skip '"' character
                        break;
                    case MODE_I :
                    case MODE_R :
                        status = QUOTE_ERROR;
                        break;
                    case MODE_S :   // end of string
                        arg_type[argc++] = MODE_S;
                        command[count] = STRING_NULL;  // put terminator on string
                        mode = MODE_U;
                        break;
                }
                break;

            case NUMBER :
                switch(mode) {
                    case MODE_U :
                        mode = MODE_I;
                        arg_pt[argc] = count;
                        break;
                    case MODE_I :
                    case MODE_R :
                    case MODE_S :
                        break;
                }
                break;

            case SEPARATOR :
                switch(mode) {
                    case MODE_U :
                    case MODE_S :
                        break;
                    case MODE_I :
                    case MODE_R :
                        arg_type[argc++] = mode;
                        command[count] = STRING_NULL;
                        mode = MODE_U;
                        break;
                }
                break;

            case DOT :
                switch(mode) {
                    case MODE_I :
                        mode = MODE_R;
                    case MODE_R :
                    case MODE_U :
                        status = DOT_ERROR;  // extra point in real value
                    case MODE_S :
                        break;
                }
                break;

            case PLUSMINUS :
                switch(mode) {
                    case MODE_U :
                        mode = MODE_I;
                        arg_pt[argc] = count;
                        break;
                    case MODE_I :
                    case MODE_R :
                        status = PLUSMINUS_ERROR;
                        break;
                    case MODE_S :
                        break;
                }
                break;

            case END :
                switch(mode) {
                    case MODE_U :
                        break;
                    case MODE_I :
                    case MODE_R :
                    case MODE_S :
                        arg_type[argc++] = mode;
                        return status;
                        break;
                }
                break;
        }   // end of SWITCH
    }  // end of FOR
    return status;
}

//***************************************************************************
// convert_tokens : convert relevant tokens to numerical values
//
int32_t convert_tokens(void) 
{
    if ((arg_type[0] != MODE_S) || (char_type[command[0]] != LETTER)) {
        return BAD_COMMAND;
    }
    for (uint32_t i=0 ; i <= argc ; i++) {
        switch (arg_type[i]) {
            case MODE_I :  // save as integer and float
                int_parameters[i] = ASCII_to_int(&command[arg_pt[i]]);
                float_parameters[i] = (float)int_parameters[i];
                break;
            case MODE_R :
                float_parameters[i] = ASCII_to_float(&command[arg_pt[i]]);
                break;
        }
    }
    return OK;
}

//***************************************************************************
error_codes_te check_command(int32_t cmd_token)
{
error_codes_te status;
uint32_t i, span;

    status = OK;

    if ((argc < cmd_limits[cmd_token].p_limits[0].parameter_min) || 
                (argc > cmd_limits[cmd_token].p_limits[0].parameter_max)) {
        status = BAD_NOS_PARAMETERS;
    } else if ((int_parameters[1] < cmd_limits[cmd_token].p_limits[1].parameter_min) || 
                (int_parameters[1] > cmd_limits[cmd_token].p_limits[1].parameter_max)) {
        status = BAD_PORT_NUMBER;
    } else {
        for (i = 2 ; i<argc ; i++) {
            if (arg_type[i] == MODE_I) {
                span = cmd_limits[cmd_token].p_limits[i].parameter_min + cmd_limits[cmd_token].p_limits[i].parameter_max;
                if (span == 0) {
                    continue;       // ignore check for this parameter
                }
                if ((int_parameters[i] < cmd_limits[cmd_token].p_limits[i].parameter_min) || 
                              (int_parameters[i] > cmd_limits[cmd_token].p_limits[i].parameter_max)) {
                    status = PARAMETER_OUTWITH_LIMITS;
                    break;
                }
            }
        }
    }
    return status;
}

