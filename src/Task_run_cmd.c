/**
 * @file Task_run_cmd.c
 * @author Jim Herd
 * @brief Read and execute command strings
 */
#include <stdio.h>
#include <stdlib.h>

#include "system.h"
#include "uart_IO.h"
#include "sys_routines.h"
#include "PCA9685.h"

#include  "Pico_IO.h"

#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "FreeRTOS.h"

int32_t parse_command (void);
int32_t convert_tokens(void);

char        command[MAX_COMMAND_LENGTH];
uint32_t    character_count;
uint32_t    argc, arg_pt[MAX_COMMAND_PARAMETERS], arg_type[MAX_COMMAND_PARAMETERS];
int         int_parameters[MAX_COMMAND_PARAMETERS];
float       float_parameters[MAX_COMMAND_PARAMETERS];

void Task_run_cmd(void *p) {

int32_t     char_count;
int32_t     status;

    FOREVER {

        char_count = uart_readline(command);
        status = parse_command();
        status = convert_tokens();

        switch(command[0]) {
            case 'p' : {  // ping command
                uart_putstring("OK\n");
                break;
            }
            default : {
                break;
            }
        }
    }
}

        // uart_putstring("1. Task_run_cmd : has been started and is waiting on a command\n");
        // uart_putstring("  2. abbccc ddddeeeee ffffff ggggggg hhhhhhhh iiiiiiiiii\n");
        // uart_putstring("    3. jjjjjjjjjj kkkkkkkkk lllllll mmmmmmm nnnnnn ooooo\n");
        // write_PCA9685_register(20,34);

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
                        // argc++;
                    } 
                }
                break;
            case NUMBER :
                if (mode == MODE_U) {
                    mode = MODE_I;
                    arg_pt[argc] = count;
                    // argc++;
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
                    //argc++;
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
                    mode = MODE_U;
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
    if ((arg_type[1] != MODE_I) || (int_parameters[1] > 63)) {
        return BAD_PORT_NUMBER;
    }
    return OK;
}
