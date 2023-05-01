/**
 * @file Task_run_cmd.c
 * @author Jim Herd
 * @brief Read and execute command strings
 */
#include <stdio.h>
#include <stdlib.h>

#include "system.h"
#include "uart_IO.h"
#include "string_IO.h"
#include "sys_routines.h"
#include "PCA9685.h"
#include "tokenizer.h"

#include  "Pico_IO.h"

#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "FreeRTOS.h"

int32_t parse_command (void);
int32_t convert_tokens(void);
error_codes_te check_command(uint32_t cmd_token);

//***************************************************************************
// Global command and parsed data

char        command[MAX_COMMAND_LENGTH];
uint32_t    character_count;
uint32_t    argc, arg_pt[MAX_ARGC], arg_type[MAX_ARGC];
int         int_parameters[MAX_ARGC];
float       float_parameters[MAX_ARGC];

//***************************************************************************
// General command limits : tested with "check_command" function
// Specific limits may be tested in the command execution code

struct command_limits_s    cmd_limits[NOS_COMMANDS] = {
    {5, 63, {{0, 5}, {-90, +90}}},
    {0, 0, {0,0}},                              // stepper,
    {0, 0, {0,0}},                               // sync,
    {0, 0, {0,0}},                               // config
    {0, 0, {0,0}},                               // info
    {3,   63,   {-255, +255}},           // ping,
};

//***************************************************************************
// Get, parse, and execute UART received command

void Task_run_cmd(void *p) {
    error_codes_te status;
    static uint32_t token;

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
            print_error(UNDEFINED_PORT, status);
            continue;
        }

        switch (token) {
            case TOKENIZER_SERVO: {
                switch (int_parameters[2]) {
                    case ABS_MOVE: {
                        set_servo_channel(int_parameters[2], int_parameters[3], int_parameters[4], false);
                        print_string("%d %d\n", int_parameters[1], OK);
                        break;
                    }
                    case SPEED_MOVE: {
                        set_servo_channel(int_parameters[2], int_parameters[3], int_parameters[4], false);
                        print_string("%d %d\n", int_parameters[1], OK);
                        break;
                    }
                    case RUN_SYNC_MOVES: {
                        set_servo_channel(int_parameters[2], int_parameters[3], int_parameters[4], false);
                        print_string("%d %d\n", int_parameters[1], OK);
                        break;
                    }
                    case STOP: {
                        set_servo_channel(int_parameters[2], int_parameters[3], int_parameters[4], false);
                        print_string("%d %d\n", int_parameters[1], OK);
                        break;
                    }
                    case STOP_ALL: {
                        set_servo_channel(int_parameters[2], int_parameters[3], int_parameters[4], false);
                        print_string("%d %d\n", int_parameters[1], OK);
                        break;
                    }
                    default: {
                        set_servo_channel(int_parameters[2], int_parameters[3], int_parameters[4], false);
                        print_string("%d %d\n", int_parameters[1], OK);
                        break;
                    }
                } 
            }
            case TOKENIZER_STEPPER: {
                break;
            }
            case TOKENIZER_SYNC: {
                break;
            }
            case TOKENIZER_CONFIG: {
                break;
            }
            case TOKENIZER_INFO: {
                break;
            }
            case TOKENIZER_PING: {
                print_string("%d %d %d\n", int_parameters[1], OK, (int_parameters[2] + 1));
                break;
                }
            default: {
                break;
                print_error(int_parameters[1],status);
            }
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

error_codes_te check_command(uint32_t cmd_token)
{
    error_codes_te status;
    uint32_t i;

    status = OK;
    if (argc != cmd_limits[cmd_token].nos_parameters) {
        status = BAD_NOS_PARAMETERS;
    } else if (int_parameters[1] > cmd_limits[cmd_token].port_max) {
        status = BAD_PORT_NUMBER;
    } else {
        for (i = 2 ; i<MAX_ARGC ; i++) {
            if (arg_type[i] == MODE_I) {
                if ((int_parameters[i] < cmd_limits[cmd_token].p_limits[i-2].parameter_min) || 
                              (int_parameters[i] > cmd_limits[cmd_token].p_limits[i-2].parameter_max)) {
                    status = PARAMETER_OUTWITH_LIMITS;
                    break;
                }
            }
        }
    }
    return status;
}
