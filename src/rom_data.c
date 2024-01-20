/**
 * 
*/

#include "pico/stdlib.h"

#include "system.h"
#include "gen4_uLCD.h"
#include "tokenizer.h"

//***************************************************************************
// ROM table to allow fast categorisation of charcaters in a command
//
// Using simple ASCII chacter set for commands this lookup table categorises 
// a charcater as one of the following types
//
// LETTER    = a-z , A-Z , '_' (others can be added)
// NUMBER    = 0->9
// PLUSMINUS = '+' and '-'
// DOT       = '.'
// TERM      = '\0'
// END       = '\n'
// OTHER     = all other characters in the 256 extended ASCII set
//
// This is uded in the command string parser in "Task_run_cmd" 

const uint8_t char_type[256] = {
    END    ,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER, SEPARATOR, END,     OTHER,    END,     OTHER,  OTHER,  OTHER,  // 00->0F
    OTHER  ,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,     OTHER,  OTHER,     OTHER,  OTHER,  OTHER,  // 10->1F
    SEPARATOR,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER, PLUSMINUS,  SEPARATOR, PLUSMINUS,    DOT,  OTHER,  // 20->2F
    NUMBER , NUMBER, NUMBER, NUMBER, NUMBER, NUMBER, NUMBER, NUMBER, NUMBER, NUMBER,  OTHER,     OTHER,  OTHER,     OTHER,  OTHER,  OTHER,  // 30->3F
    OTHER  , LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER,    LETTER, LETTER,    LETTER, LETTER, LETTER,  // 40->4F
    LETTER , LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER,     OTHER,  OTHER,     OTHER,  OTHER, LETTER,  // 50->5F
    OTHER  , LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER,    LETTER, LETTER,    LETTER, LETTER, LETTER,  // 40->4F
    LETTER , LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER,     OTHER,  OTHER,     OTHER,  OTHER,  OTHER,  // 70->7F

     OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,     OTHER,  OTHER,     OTHER,  OTHER,  OTHER,  // 80->8F
     OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,     OTHER,  OTHER,     OTHER,  OTHER,  OTHER,  // 90->9F
     OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,     OTHER,  OTHER,     OTHER,  OTHER,  OTHER,  // A0->AF
     OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,     OTHER,  OTHER,     OTHER,  OTHER,  OTHER,  // B0->BF
     OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,     OTHER,  OTHER,     OTHER,  OTHER,  OTHER,  // C0->CF
     OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,     OTHER,  OTHER,     OTHER,  OTHER,  OTHER,  // D0->DF
     OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,     OTHER,  OTHER,     OTHER,  OTHER,  OTHER,  // E0->EF
     OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,     OTHER,  OTHER,     OTHER,  OTHER,  OTHER,  // F0->FF
};

struct token_list_s commands[] = {
    {"servo",   TOKENIZER_SERVO},
    {"stepper", TOKENIZER_STEPPER},
    {"sync",    TOKENIZER_SYNC},
    {"set",     TOKENIZER_SET},
    {"get",     TOKENIZER_GET},
    {"ping",    TOKENIZER_PING},
    {"delay",   TOKENIZER_TDELAY},
    {"display", TOKENIZER_DISPLAY},
};

struct error_list_s errors[] = {
    {OK,                 "OK"},
    {LETTER_ERROR,       "Parse : Letter in number error"},
    {DOT_ERROR,          "Parse : Point in number error"},
    {PLUSMINUS_ERROR,    "Parse : +/- in wrong place"},
    {BAD_COMMAND,        "Unknown command"},
    {BAD_PORT_NUMBER,    "Bad PORT number"},
    {BAD_NOS_PARAMETERS, "Wrong number of parameters"},
    {BAD_BASE_PARAMETER, "printf int base not 10 or 16"},
    {PARAMETER_OUTWITH_LIMITS, "A parameter is outwith its limits"},
    {BAD_SERVO_COMMAND,  "unknown servo command"},
    {STEPPER_CALIBRATE_FAIL, "Stepper motor calibration has failed"},
    {BAD_STEPPER_COMMAND,  "unknown stepper motor command"},
};

//***************************************************************************
// set of trapezoidal sm_profiles for stepper motor moves

 struct sm_profile_s  sequences[NOS_PROFILES] = {
    { 7,
        {   {SM_ACCEL,1,12},{SM_ACCEL,1,9},{SM_ACCEL,1,6,},    // fast speed
            {SM_COAST,1,3},
            {SM_DECEL,1,6},{SM_DECEL,1,9},{SM_DECEL,1,12},
            {SM_END,0,0},
        }
    }
 };

//***************************************************************************
// General command limits : tested with "check_command" function
// Specific limits may be tested in the command execution code

struct command_limits_s    cmd_limits[NOS_COMMANDS] = {     
// paramter        NOS_PAR     1        2       3          4         5
    [0].p_limits = {{5, 6}, {0, 63}, {0, 8}, {0, 15}, {-90, +90}, {1, 1000}},   // servo
    [1].p_limits = {{5, 6}, {0, 63}, {0, 4}, {0, 0}, {-333, +333}},             // stepper
    [2].p_limits = {{2, 2}, {0, 63}, {0,0}},                                    // sync
    [3].p_limits = {{0, 0}, {0,  0}, {0,0}},                                    // config
    [4].p_limits = {{3, 3}, {0, 63}, {0,5}},                                    // info
    [5].p_limits = {{3, 3}, {0, 63}, {-255, +255}},                             // ping,
    [6].p_limits = {{3, 3}, {0, 63}, {0, 50000}},                               // delay
    [7].p_limits = {{4, 5}, {0, 63}, {0, 4}},                                   // display
};

//==============================================================================
// servo data with initial values

struct servo_data_s     servo_data[NOS_SERVOS] = {
    {DORMANT, false, SERVO, 0, 0, 0, 45, false, -25, +25,  0, 10},
    {DORMANT, false, SERVO, 0, 0, 0, 45, false, -45, +45, 10, 10},
    {DORMANT, false, SERVO, 0, 0, 0, 45, false, -25, +25, 20, 10},
    {DORMANT, false, SERVO, 0, 0, 0, 45, false, -30, +30, 30, 10},
    {DORMANT, false, SERVO, 0, 0, 0, 45, false, -25, +25, 40, 10},
    {DORMANT, false, SERVO, 0, 0, 0, 45, false, -45, +45, 50, 10},
    {DORMANT, false, SERVO, 0, 0, 0, 45, false, -25, +25, 50, 10},
    {DORMANT, false, SERVO, 0, 0, 0, 45, false, -30, +30, 50, 10},
    {DORMANT, false, MOTOR, 0, 0, 0, 45, false, -45, +45, 60, 10},
};

//==============================================================================
// Data relevant to the 8 commands used to communicate with the
// 4D Systems Gen4 Diablo16 based display

struct display_cmd_reply_data_s    display_cmd_info[NOS_GEN4_uLCD_CMDS] = {
	{ HOST_TO_DISPLAY,  4, NAK_REPORT},	    // 0 = READ_OBJ
	{ HOST_TO_DISPLAY,  6, ACK_NAK},	    // 1 = WRITE_OBJ
	{ HOST_TO_DISPLAY,  4, ACK_NAK},	    // 2 = WRITE_STR
	{ HOST_TO_DISPLAY, -1, ACK_NAK},	    // 3 = WRITE_STRU
	{ HOST_TO_DISPLAY,  3, ACK_NAK},	    // 4 = WRITE_CONTRAST
	{ DISPLAY_TO_HOST,  6, NAK_REPLY},	    // 5 = REPORT_OBJ
	{ HOST_TO_DISPLAY,  0, ILLEGAL},	    // 6 = illegal op
	{ DISPLAY_TO_HOST,  6, NAK_REPLY},	    // 7 = REPORT_EVENT
};

//==============================================================================
// Data about the forms and touch button used on the 4D Systems display.

struct form_data_s   form_data[MAX_NOS_FORMS] = {
    {false, 1, {{1,0,0}}},
    {false, 2, {{2,0,0}, {3,0,0}}}
};



