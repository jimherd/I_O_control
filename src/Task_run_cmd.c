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
struct servo_data_s *servo_pt;
error_codes_te status;
static int32_t token;
bool           reply_done;
int32_t sm_number;
int32_t target_step_count;

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
                switch (int_parameters[2]) {
                    case ABS_MOVE: 
                        set_servo_move( int_parameters[3], MOVE, int_parameters[4], false);
                        break;
                    case ABS_MOVE_SYNC: 
                        set_servo_move( int_parameters[3], MOVE, int_parameters[4], true);
                        break;
                    case SPEED_MOVE: 
                        set_servo_speed_move(int_parameters[3], TIMED_MOVE, int_parameters[4], int_parameters[5], false);
                        break;
                    case SPEED_MOVE_SYNC: 
                        set_servo_speed_move(int_parameters[3], TIMED_MOVE, int_parameters[4], int_parameters[5], true);
                        break;
                    case RUN_SYNC_MOVES: 
                        set_servo_move(int_parameters[2], int_parameters[3], int_parameters[4], false);
                        break;
                    case STOP:
                        set_servo_move(DORMANT, int_parameters[3], int_parameters[4], false);  
                        break;
                    case STOP_ALL: 
                        set_servo_move(int_parameters[2], int_parameters[3], int_parameters[4], false);
                        break;
                    default:
                        status = BAD_SERVO_COMMAND;
                        break;
                }  // end of inner switch
                print_string("%d %d\n", int_parameters[1], status);
                reply_done = true;
                break;
            case TOKENIZER_STEPPER: 
    #ifndef IGNORE_SM_CALIBRATION
                if (stepper_data[int_parameters[5]].error != OK) {  // ensure motor is not in an error state
                    status = stepper_data[int_parameters[5]].error;
                    break;
                }
    #endif
                switch (int_parameters[2]) {
                    case SM_REL_MOVE : 
                    case SM_REL_MOVE_SYNC :
                        sm_number = int_parameters[5];
                        if (abs(int_parameters[4]) < MIN_STEP_MOVE ) {
                            status = SM_MOVE_TOO_SMALL;
                            break;
                        }
                        if (int_parameters[4] < 0) {
                            stepper_data[sm_number].direction = ANTI_CLOCKWISE;
                        } else {
                            stepper_data[sm_number].direction = CLOCKWISE;
                        }
                        if (stepper_data[sm_number].flip_direction == true) {
                            FLIP_BOOLEAN(stepper_data[sm_number].direction);
                        }
                        target_step_count = stepper_data[sm_number].current_step_count + int_parameters[4];
                        if ((target_step_count < 0) || (target_step_count > stepper_data[sm_number].max_step_travel)) {
                            status = BAD_STEP_VALUE;
                            break;
                        }
                        stepper_data[sm_number].target_step_count = target_step_count;
                        stepper_data[sm_number].sm_profile = sm_number;
                        stepper_data[sm_number].coast_step_count = abs(int_parameters[4]) - (sequences[sm_number].nos_sm_cmds - 1);
                        stepper_data[sm_number].cmd_index = 0;
                        if (int_parameters[2] == SM_REL_MOVE) {
                            stepper_data[sm_number].state = M_INIT;
                        } else {
                            stepper_data[sm_number].state = M_SYNC;
                        }
                        break;
                    case SM_ABS_MOVE :
                        sm_number = int_parameters[5];
                        if ((int_parameters[4] < 0) || (int_parameters[4] > stepper_data[sm_number].max_step_travel)) {
                            status = BAD_STEP_VALUE;
                            break;
                        }
                        target_step_count = int_parameters[4] - stepper_data[sm_number].current_step_count;
                        if (target_step_count < 0) {
                            stepper_data[sm_number].direction = ANTI_CLOCKWISE;
                        } else {
                            stepper_data[sm_number].direction = CLOCKWISE;
                        }
                        if (stepper_data[sm_number].flip_direction == true) {
                            FLIP_BOOLEAN(stepper_data[sm_number].direction);
                        }
                        stepper_data[sm_number].target_step_count = target_step_count;
                        stepper_data[sm_number].sm_profile = sm_number;
                        stepper_data[sm_number].coast_step_count = abs(target_step_count) - (sequences[sm_number].nos_sm_cmds - 1);
                        stepper_data[sm_number].cmd_index = 0;
                        if (int_parameters[2] == SM_ABS_MOVE) {
                            stepper_data[sm_number].state = M_INIT;
                        } else {
                            stepper_data[sm_number].state = M_SYNC;
                        }
                        break;
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
                    if (stepper_data[i].state == M_SYNC) {
                        stepper_data[i].state = M_INIT;
                    }
                }
                break;
            case TOKENIZER_CONFIG: 
                break;
            case TOKENIZER_INFO:
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
            case TOKENIZER_TDELAY:
                vTaskDelay(int_parameters[2]);
                break;
            default: 
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
                if ((mode == MODE_I) || (mode == MODE_R)) {
                    status = LETTER_ERROR;
                } else {
                    if (mode == MODE_U) {
                        mode = MODE_S;
                        arg_pt[argc] = count;
                        // argc++; //
                    } 
                }
                break;
            case NUMBER :
                if (mode == MODE_U) {
                    mode = MODE_I;
                    arg_pt[argc] = count;
                    // argc++;  //
                } 
                break;
            case SEPARATOR :
                if (mode != MODE_U) {
                    arg_type[argc++] = mode;
                    command[count] = STRING_NULL;
                    mode = MODE_U;
                }
                break;
            case DOT :
                if (mode == MODE_I) {
                    mode = MODE_R;
                } else {
                    if ((mode == MODE_R) || (mode == MODE_U)) {
                        status = DOT_ERROR;  // extra point in real value
                    } 
                }
                break;
            case PLUSMINUS :
                if (mode == MODE_U) {
                    mode = MODE_I;
                    arg_pt[argc] = count;
                    // argc++;   //
                } else {
                    if ((mode == MODE_I) || (mode == MODE_R)) {
                        status = PLUSMINUS_ERROR;
                    }
                }
                break;
            case NULTERM :
                if (mode != MODE_U) {
                    arg_type[argc++] = mode;
                    mode = MODE_U;
                }
                break;
            case END :
                if (mode != MODE_U) {
                    arg_type[argc++] = mode;
                    //mode = MODE_U;
                    return status;
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
    // if ((arg_type[1] != MODE_I) || (int_parameters[1] > 63)) {
    //     return BAD_PORT_NUMBER;
    // }
    return OK;
}

//***************************************************************************
error_codes_te check_command(int32_t cmd_token)
{
error_codes_te status;
uint32_t i;

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

//==============================================================================
// Archive : delete after system tests
//
// //***************************************************************************
// // General command limits : tested with "check_command" function
// // Specific limits may be tested in the command execution code

// struct command_limits_s    cmd_limits[NOS_COMMANDS] = {
//     [0].p_limits = {{5, 6}, {0, 63}, {0, 5}, {0, 15}, {-90, +90}, {1, 1000}},   // servo
//     [1].p_limits = {{5, 6}, {0, 63}, {0,0}, {0, 0}, {-333, +333}},              // stepper
//     [2].p_limits = {{2, 2}, {0, 63}, {0,0}},                         // sync
//     [3].p_limits = {{0, 0}, {0,  0}, {0,0}},                         // config
//     [4].p_limits = {{0, 0}, {0,  0}, {0,0}},                         // info
//     [5].p_limits = {{3, 3}, {0, 63}, {-255, +255}},                  // ping,
// };
