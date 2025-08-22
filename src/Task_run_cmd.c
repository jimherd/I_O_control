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
#include  "neopixel.h"
#include  "gen4_uLCD.h"

//***************************************************************************
// Function prototypes

error_codes_te parse_command (void);
error_codes_te convert_tokens(void);
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
uint32_t                current_form, new_form, old_form, result, i, value, pressed_state;

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
            print_error(int_parameters[PORT_INDEX], status);
            continue;
        }

        reply_done = false;
        status = OK;
        switch (token) {
            case TOKENIZER_SYS:
                switch(int_parameters[2]) {  // SYS_SUB_CMD_INDEX
                    case SOFT_RESET :    
                        // special : NO RETURN FROM THIS COMMAND
                        // Therefor send OK response BEFORE executing command to
                        // ensure that remote comuputer does not enter a hang state
                        print_string("%d %d\n", int_parameters[PORT_INDEX], status);
                        software_reset();
                    default :
                        status = -9999;
                        break;
                break;
                }

            case TOKENIZER_SERVO: 
                switch (int_parameters[SERVO_SUB_CMD_INDEX]) {
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
                        status = set_servo_move(int_parameters[SERVO_SUB_CMD_INDEX], int_parameters[SERVO_NUMBER_INDEX], int_parameters[SERVO_ANGLE_INDEX], false);
                        break;
                    case STOP:
                        status = set_servo_move(int_parameters[SERVO_SUB_CMD_INDEX], int_parameters[SERVO_NUMBER_INDEX], int_parameters[SERVO_ANGLE_INDEX], false);  
                        break;
                    case STOP_ALL: 
                        status = set_servo_move(int_parameters[SERVO_SUB_CMD_INDEX], int_parameters[SERVO_NUMBER_INDEX], int_parameters[SERVO_ANGLE_INDEX], false);
                        break;
                    case ENABLE :
                        status = set_servo_state(int_parameters[SERVO_NUMBER_INDEX], DISABLED, int_parameters[SERVO_ANGLE_INDEX]);
                        break;
                    default:
                        status = BAD_SERVO_COMMAND;
                        break;
                }  // end of inner switch
                print_string("%d %d\n", int_parameters[PORT_INDEX], status);
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
                switch (int_parameters[STEP_MOTOR_SUB_CMD_INDEX]) { 

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
                        if (int_parameters[STEP_MOTOR_SUB_CMD_INDEX] == SM_REL_MOVE) {
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
                print_string("%d %d\n", int_parameters[PORT_INDEX], status);
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
                        print_string("%d %d %d %d\n", int_parameters[PORT_INDEX], OK, NOS_SERVOS, NOS_STEPPERS);
                        reply_done = true;
                        break;
                    case SERVO_INFO:
                        print_string("%d %d\n", int_parameters[PORT_INDEX], OK);
                        reply_done = true;
                        break;
                    case STEPPER_INFO:
                        print_string("%d %d\n", int_parameters[PORT_INDEX], OK);
                        reply_done = true;
                        break;
                    default:
                        break;
                }
                break;

            case TOKENIZER_PING: 
                print_string("%d %d %d\n", int_parameters[PORT_INDEX], OK, (int_parameters[PING_VALUE_INDEX] + 1));
                reply_done = true;
                break;

            case TOKENIZER_DISPLAY:
                switch (int_parameters[DISPLAY_SUB_CMD_INDEX]) {
                    case SET_uLCD_FORM:
                        new_form = int_parameters[DISPLAY_FORM_INDEX];
                        if (new_form > NOS_FORMS) {
                            status = GEN4_uLCD_CMD_BAD_FORM_INDEX;
                            break;
                        } 
                        // scan any switches marked to be scanned and put their 
                        // values in the 'form_data' structure
                        status = scan_switches(get_uLCD_active_form(), &result);
                        if (status != OK) {
                            break;
                        }
                        new_form = int_parameters[DISPLAY_FORM_INDEX];
                        if (new_form > NOS_FORMS) {
                            status = GEN4_uLCD_CMD_BAD_FORM_INDEX;
                            break;
                        } 
                        status = change_uLCD_form(new_form);
                        if (status != OK) {
                            break;
                        }
                        // clear button states
                        for (i = 0; i < nos_object[new_form].nos_buttons; i++) {
                            clear_button_state(new_form, i);
                        }
                        // update any strings
                        for (i = 0; i < nos_object[new_form].nos_strings; i++) {
                            status = gen4_uLCD_WriteString(form_data[new_form].strings[i].global_object_id,
                                              &form_data[new_form].strings[i].string[0]);
                        }
                        break;

                    case GET_uLCD_FORM:
                        print_string("%d %d %d\n",int_parameters[PORT_INDEX], OK,  get_uLCD_active_form());
                        reply_done = true;
                        break;

                    case SET_uLCD_CONTRAST:
                        if (int_parameters[DISPLAY_CONTRAST_INDEX] < 0 || int_parameters[DISPLAY_CONTRAST_INDEX] > 100) {
                            status = GEN4_uLCD_WRITE_CONTRAST_BAD_VALUE;
                            break;
                        }
                        status = gen4_uLCD_WriteContrast(int_parameters[DISPLAY_CONTRAST_INDEX]);
                        break;

                    case READ_uLCD_BUTTON:   // read from 'form_data' structure
                        current_form = get_uLCD_active_form();
                        if (int_parameters[DISPLAY_FORM_INDEX] != current_form ) {
                            status = GEN4_uLCD_BUTTON_FORM_INACTIVE;
                            break;
                        } 
                        pressed_state = form_data[current_form].buttons[int_parameters[DISPLAY_LOCAL_ID_INDEX]].button_state;
                        value = form_data[current_form].buttons[int_parameters[DISPLAY_LOCAL_ID_INDEX]].button_value;
                        print_string("%d %d %d %d\n", int_parameters[PORT_INDEX], status, value, pressed_state);
                        reply_done = true;
                        break;

                    case READ_uLCD_SWITCH:   // read from 'form_data' structure
                        current_form = get_uLCD_active_form();
                        if (int_parameters[DISPLAY_FORM_INDEX] != current_form ) {
                            status = GEN4_uLCD_BUTTON_FORM_INACTIVE;
                            break;
                        } 
                        if (int_parameters[DISPLAY_DATA_SOURCE_INDEX] == SRC_HARDWARE) {    // read from display hardware
                            int32_t object_type = form_data[current_form].switches[int_parameters[DISPLAY_LOCAL_ID_INDEX]].object_type;
                            int32_t global_object_id = form_data[current_form].switches[int_parameters[DISPLAY_LOCAL_ID_INDEX]].global_object_id;
                            status = gen4_uLCD_ReadObject(object_type, 
                                                            global_object_id, 
                                                            &result);
                            if (status != OK) {
                                break;
                            }
                            // log result 
		                    form_data[current_form].switches[int_parameters[DISPLAY_LOCAL_ID_INDEX]].switch_value = result;
                        } else {      // read from 'form_data' structure
                            result = form_data[current_form].switches[int_parameters[DISPLAY_LOCAL_ID_INDEX]].switch_value;
                        }
                        print_string("%d %d %d\n", int_parameters[PORT_INDEX], status, result);
                        reply_done = true;
                        break;

                    case READ_uLCD_OBJECT:  //read from display hardware
                        status = gen4_uLCD_ReadObject(int_parameters[DISPLAY_OBJECT_TYPE_INDEX],
                                                      int_parameters[DISPLAY_GLOBAL_ID_INDEX],
                                                      &result);
                        print_string("%d %d %d\n", int_parameters[PORT_INDEX], status, result);
                        break;

                    case WRITE_uLCD_STRING:
                        current_form = get_uLCD_active_form();
                        if (int_parameters[DISPLAY_FORM_INDEX] != current_form ) {
                            status = GEN4_uLCD_BUTTON_FORM_INACTIVE;
                            break;
                        } 
                        status = gen4_uLCD_WriteString(form_data[current_form].strings[int_parameters[DISPLAY_LOCAL_ID_INDEX]].global_object_id,
                                                       &command[arg_pt[DISPLAY_STRING_INDEX]]);

                    case WRITE_uLCD_OBJECT :   // raw write to a screen object
                            status = gen4_uLCD_WriteObject(int_parameters[DISPLAY_OBJECT_TYPE_INDEX], 
                                                           int_parameters[DISPLAY_GLOBAL_ID_INDEX], 
                                                           int_parameters[DISPLAY_WRITE_VALUE_INDEX]);
                            break;

                    case SCAN_uLCD_BUTTON_PRESSES:
                        current_form = get_uLCD_active_form();
                        if (int_parameters[DISPLAY_FORM_INDEX] != current_form ) {
                            status = GEN4_uLCD_BUTTON_FORM_INACTIVE;
                            break;
                        } 
                        for (i=0 ; i < nos_object[current_form].nos_buttons; i++) {
                            if (form_data[current_form].buttons[i].object_mode != OBJECT_SCAN_ENABLED) {
                                break;
                            }
                            if (form_data[current_form].buttons[i].button_state == PRESSED) {
                                print_string("%d %d %d %d\n", int_parameters[PORT_INDEX] , OK, i, form_data[current_form].buttons[i].time_high);
                                reply_done = true;
                                // clear state to button data
                                clear_button_state(current_form, i);
                                break;
                            }
                        }
                        if (reply_done != true) {
                            print_string("%d %d %d\n", int_parameters[PORT_INDEX], OK, -1);
                            reply_done = true;
                        }
                        break;

                    case SCAN_uLCD_SWITCHES:
                        status = scan_switches(get_uLCD_active_form(), &result);
                        if (status == OK) {
                            print_string("%d %d %d\n", int_parameters[PORT_INDEX], OK, result);
                            reply_done = true;
                            break;
                        }
                        break;
                        
                    default:
                        status = GEN4_UNKNOWN_DISPLAY_SUB_COMMAND;
                        break;
                }
                break;

            case TOKENIZER_NEOPIXEL:
                switch (int_parameters[NEOPIXEL_SUB_CMD_INDEX]) {
                    case NP_SET_PIXEL_ON:
                        set_neopixel_on(int_parameters[3], int_parameters[4]);
                        break;
                    case NP_SET_PIXEL_OFF:
                        set_neopixel_on(int_parameters[3], N_BLACK);
                        break;
                    case NP_SET_PIXEL_FLASH:
                        set_neopixel_flash(int_parameters[3], int_parameters[4], int_parameters[5],
                                           int_parameters[6], int_parameters[7]);
                        break;
                    case NP_BLANK_ALL:
                        for (int i = 0; i < NOS_NEOPIXELS; i++) {
                            set_neopixel_on(int_parameters[3], N_BLACK);
                        }
                        break;
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
//
// defines modes as scan progresses : U=undefined, I=integer, R=real, S=string
//

error_codes_te parse_command (void) 
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
                        mode = MODE_W;
                        arg_pt[argc] = count;
                        break;
                    case MODE_I :
                    case MODE_R :
                        status = LETTER_ERROR;
                        break;
                    case MODE_W :   
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
                    case MODE_W :
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
                    case MODE_W :
                    case MODE_S :
                        break;
                }
                break;

            case SEPARATOR :
                switch(mode) {
                    case MODE_U :
                    case MODE_S :
                        break;
                    case MODE_W :
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
                    case MODE_W :
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
                    case MODE_W :
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
                    case MODE_W :
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
error_codes_te convert_tokens(void) 
{
    if ((arg_type[0] != MODE_W) || (char_type[command[0]] != LETTER)) {
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



