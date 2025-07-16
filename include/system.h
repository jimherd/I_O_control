/**
 * @file    system.h
 * @author  Jim Herd 
 * @brief   General constants
 */

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include    <stdint.h>

#include    "pico/stdlib.h"
#include    "Pico_IO.h"

#include    "FreeRTOS.h"
#include    "semphr.h"
#include    "event_groups.h"

//#include    "gen4_uLCD.h"

//==============================================================================
// Version number
//==============================================================================
#define     MAJOR_VERSION       0
#define     MINOR_VERSION       1
#define     PATCH_VERSION       0

//==============================================================================
// Complie time configuration
//==============================================================================

#define     IGNORE_SM_CALIBRATION

//==============================================================================
// Macros
//==============================================================================

#define  FLIP_BOOLEAN(x)   ((x) ^= 1)

#define     START_PULSE         gpio_put(LOG_PIN, 1)
#define     STOP_PULSE          gpio_put(LOG_PIN, 0)

#define     FOREVER     for(;;)
#define     HANG        for(;;)

#define     ATTRIBUTE_PACKED     __attribute__ ((__packed__))

#define lowByte16(x)      ((int8_t)((x) & 0xFF))
#define highByte16(x)     ((int8_t)(((x) >> 8) & 0xFF))

//==============================================================================
// Constants
//==============================================================================
// CPU

//#define     CPU_CLOCK_FREQUENCY         125000000   // 125MHz
   #define     CPU_CLOCK_FREQUENCY         200000000   // 200MHz

//==============================================================================
// Useful times

#define     HALF_SECOND      (500/portTICK_PERIOD_MS)
#define     ONE_SECOND       (1000/portTICK_PERIOD_MS)
#define     TWO_SECONDS      (2000/portTICK_PERIOD_MS)

//==============================================================================
// Useful general system structures

#define     MAX_STRING_SIZE   48

struct string_buffer {
    char        buffer[MAX_STRING_SIZE];
    uint32_t    char_pt;
    bool        full;
};

//==============================================================================
// error codes

typedef enum  {
    OK                               = 0,
    LETTER_ERROR                     = -100,
    DOT_ERROR                        = -101,
    PLUSMINUS_ERROR                  = -102,
    BAD_COMMAND                      = -103,
    BAD_PORT_NUMBER                  = -104,
    BAD_NOS_PARAMETERS               = -105,
    BAD_BASE_PARAMETER               = -106,
    PARAMETER_OUTWITH_LIMITS         = -107,
    BAD_SERVO_COMMAND                = -108,
    STEPPER_CALIBRATE_FAIL           = -109,
    BAD_STEPPER_COMMAND              = -110,
    BAD_STEP_VALUE                   = -111,
    MOVE_ON_UNCALIBRATED_MOTOR       = -112,
    EXISTING_FAULT_WITH_MOTOR        = -113,
    SM_MOVE_TOO_SMALL                = -114,
    LIMIT_SWITCH_ERROR               = -115,
    UNKNOWN_STEPPER_MOTOR_STATE      = -116,
    STEPPER_BUSY                     = -117,
    SERVO_BUSY                       = -118,
    GEN4_uLCD_NOT_DETECTED           = -119,
    GEN4_uLCD_WRITE_OBJ_FAIL         = -120,
    GEN4_uLCD_WRITE_OBJ_TIMEOUT      = -121,
    GEN4_uLCD_WRITE_CONTRAST_FAIL    = -122,
    GEN4_uLCD_WRITE_CONTRAST_TIMEOUT = -123,   
    GEN4_uLCD_READ_OBJ_FAIL          = -124,
    GEN4_uLCD_READ_OBJ_TIMEOUT       = -125,
    GEN4_uLCD_CMD_BAD_FORM_INDEX     = -126,
    GEN4_uLCD_WRITE_STR_TOO_BIG      = -127,
    GEN4_uLCD_WRITE_STRING_FAIL      = -128,
    GEN4_uLCD_WRITE_STRING_TIMEOUT   = -129,
    GEN4_uLCD_BUTTON_FORM_INACTIVE   = -130,
    GEN4_uLCD_EXPECTED_BUTTON_OBJECT = -131,
    GEN4_uLCD_BUTTON_OBJECT_NOT_USED = -132,
    GEN4_uLCD_STRING_FORM_INACTIVE   = -133,
    GEN4_uLCD_SWITCH_OBJECT_NOT_USED = -134,
    QUOTE_ERROR                      = -135,
} error_codes_te;



//==============================================================================
// Serial comms port (UART)
//==============================================================================

#define UART0_ID         uart0
#define UART0_TX_PIN     GP0
#define UART0_RX_PIN     GP1

#define UART0_BAUD_RATE 115200

#define LINE_AVAILABLE  0   // bit in eventgroup_uart_IO

#define     RETURN      '\r'
#define     NEWLINE     '\n'
#define     TAB         '\t'
#define     SPACE       ' '
#define     MINUS       '-'
#define     CHAR_0      '0'
#define     STRING_NULL '\0'
#define     PERCENT     '%'

#define     MAX_STRING_LENGTH       80
#define     MAX_GEN4_uLCD_WRITE_STR_SIZE    40

#define     MAX_COMMAND_LENGTH      100

#define     MAX_ARGC  8

enum modes_e {MODE_U, MODE_I, MODE_R, MODE_W, MODE_S} ;  // defines modes as scan progresses

enum {LETTER, NUMBER, DOT, PLUSMINUS, END, QUOTE, SEPARATOR, OTHER};

enum {BASE_10 = 10, BASE_16 = 16};
enum {UPPER_CASE, LOWER_CASE};

//==============================================================================
// Serial display port (UART) - 4D System display
//==============================================================================

#define UART1_ID           uart1
#define DISPLAY_RESET_PIN  GP3
#define UART1_TX_PIN       GP4
#define UART1_RX_PIN       GP5

//#define UART1_BAUD_RATE   200000
#define UART1_BAUD_RATE   115200



typedef enum {SET_FORM, GET_FORM, SET_CONTRAST, READ_BUTTON, WRITE_STRING} display_commands_te;

#define     NOS_FORMS       3
#define     NOS_BUTTONS     3



#define MAX_GEN4_uLCD_EVENTS    	16    // MUST be a power of 2



#define		GEN4_uLCD_NOS_PINGS		1   // to set display

typedef enum {
	GEN4_uLCD_READ_OBJ,
	GEN4_uLCD_WRITE_OBJ,
	GEN4_uLCD_WRITE_STR,
	GEN4_uLCD_WRITE_STRU,
	GEN4_uLCD_WRITE_CONTRAST,
	GEN4_uLCD_REPORT_OBJ,
	GEN4_uLCD_REPORT_EVENT = 7,
} gen4_uLCD_Command_te;

typedef enum {
	GEN4_uLCD_OBJ_DIPSW,
	GEN4_uLCD_OBJ_KNOB,
	GEN4_uLCD_OBJ_ROCKERSW,
	GEN4_uLCD_OBJ_ROTARYSW,
	GEN4_uLCD_OBJ_SLIDER,
	GEN4_uLCD_OBJ_TRACKBAR,
	GEN4_uLCD_OBJ_WINBUTTON,        // 6, (0x06)
	GEN4_uLCD_OBJ_ANGULAR_METER,
	GEN4_uLCD_OBJ_COOL_GAUGE,
	GEN4_uLCD_OBJ_CUSTOM_DIGITS,
	GEN4_uLCD_OBJ_FORM,
	GEN4_uLCD_OBJ_GAUGE,
	GEN4_uLCD_OBJ_IMAGE,
	GEN4_uLCD_OBJ_KEYBOARD,
	GEN4_uLCD_OBJ_LED,
	GEN4_uLCD_OBJ_LED_DIGITS,
	GEN4_uLCD_OBJ_METER,
	GEN4_uLCD_OBJ_STRINGS,
	GEN4_uLCD_OBJ_THERMOMETER,
	GEN4_uLCD_OBJ_USER_LED,
	GEN4_uLCD_OBJ_VIDEO,
	GEN4_uLCD_OBJ_STATIC_TEXT,
	GEN4_uLCD_OBJ_SOUND,
	GEN4_uLCD_OBJ_TIMER,
	GEN4_uLCD_OBJ_SPECTRUM,
	GEN4_uLCD_OBJ_SCOPE,
	GEN4_uLCD_OBJ_TANK,
	GEN4_uLCD_OBJ_USERIMAGES,
	GEN4_uLCD_OBJ_PINOUTPUT,
	GEN4_uLCD_OBJ_PININPUT,
	GEN4_uLCD_OBJ_4DBUTTON,
	GEN4_uLCD_OBJ_ANIBUTTON,
	GEN4_uLCD_OBJ_COLORPICKER,
	GEN4_uLCD_OBJ_USERBUTTON,
	GEN4_uLCD_OBJ_MAGIC_RESERVED,
	GEN4_uLCD_OBJ_SMARTGAUGE,
	GEN4_uLCD_OBJ_SMARTSLIDER,
	GEN4_uLCD_OBJ_SMARTKNOB,
	GEN4_uLCD_OBJ_ILED_DIGITS_H,
	GEN4_uLCD_OBJ_IANGULAR_METER,
	GEN4_uLCD_OBJ_IGAUGE,
	GEN4_uLCD_OBJ_ILABELB,
	GEN4_uLCD_OBJ_IUSER_GAUGE,
	GEN4_uLCD_OBJ_IMEDIA_GAUGE,
	GEN4_uLCD_OBJ_IMEDIA_THERMOMETER,
	GEN4_uLCD_OBJ_ILED,
	GEN4_uLCD_OBJ_IMEDIA_LED,
	GEN4_uLCD_OBJ_ILED_DIGITS_L,
	GEN4_uLCD_OBJ_ILED_DIGITS,
	GEN4_uLCD_OBJ_INEEDLE,
	GEN4_uLCD_OBJ_IRULER,
	GEN4_uLCD_OBJ_ILED_DIGIT,
	GEN4_uLCD_OBJ_IBUTTOND,
	GEN4_uLCD_OBJ_IBUTTONE,
	GEN4_uLCD_OBJ_IMEDIA_BUTTON,
	GEN4_uLCD_OBJ_ITOGGLE_INPUT,
	GEN4_uLCD_OBJ_IDIAL,
	GEN4_uLCD_OBJ_IMEDIA_ROTARY,
	GEN4_uLCD_OBJ_IROTARY_INPUT,
	GEN4_uLCD_OBJ_ISWITCH,
	GEN4_uLCD_OBJ_ISWITCHB,         // 59,(0x3B)
	GEN4_uLCD_OBJ_ISLIDERE,
	GEN4_uLCD_OBJ_IMEDIA_SLIDER,
	GEN4_uLCD_OBJ_ISLIDERH,
	GEN4_uLCD_OBJ_ISLIDERG,
	GEN4_uLCD_OBJ_ISLIDERF,
	GEN4_uLCD_OBJ_ISLIDERD,
	GEN4_uLCD_OBJ_ISLIDERC,
	GEN4_uLCD_OBJ_ILINEAR_INPUT
} gen4_uLCD_Object_te;

//
// INDEX codes for objects on the Gen4 uLCD display
//
enum {
	GEN4_uLCD_WINBUTTON0, 
	GEN4_uLCD_WINBUTTON1, 
	GEN4_uLCD_WINBUTTON2, 
	GEN4_uLCD_WINBUTTON3, 
	GEN4_uLCD_WINBUTTON4, 
	GEN4_uLCD_WINBUTTON5,  
	GEN4_uLCD_WINBUTTON6,  
	GEN4_uLCD_WINBUTTON7,  
};

enum {
	GEN4_uLCD_FORM0,  
	GEN4_uLCD_FORM1, 
	GEN4_uLCD_FORM2, 
	GEN4_uLCD_FORM3, 	
	GEN4_uLCD_FORM4, 
	GEN4_uLCD_FORM5, 
	GEN4_uLCD_FORM6, 
    GEN4_uLCD_FORM7,
};

enum {
	GEN4_uLCD_STRING0,
	GEN4_uLCD_STRING1,
	GEN4_uLCD_STRING2,
	GEN4_uLCD_STRING3,
	GEN4_uLCD_STRING4,	
	GEN4_uLCD_STRING5,
	GEN4_uLCD_STRING6,	
	GEN4_uLCD_STRING7,
};

//==============================================================================
//I2C port
//==============================================================================

#define I2C_PORT    i2c0
#define I2C_SDA     GP8
#define I2C_SCL     GP9

//==============================================================================
// log and blink pins
//==============================================================================

#define LED_PIN     PICO_DEFAULT_LED_PIN
#define LOG_PIN     GP2
#define BLINK_PIN   LED_PIN

//==============================================================================
//servo motor interface (PCA9685A)
//==============================================================================

#define     PCA9685_address     0x40

#define		PCA9685_servo_frequency		 50  // hertz
#define		PCA9685_50Hz_PRE_SCALER		138  // TUNED : calc = 123
  // refer to datasheet for calculation

#define		SERVO_TRIM_MIN		110
#define     SERVO_TRIM_MAX		590

#define		MID_POINT_COUNT		307
#define		COUNT_1mS			205
#define		MAX_ANGLE			 90

enum {R_EYE_LR, R_EYE_UD, R_EYE_LID, R_EYE_BROW, L_EYE_LR, L_EYE_UD, L_EYE_LID, L_EYE_BROW, MOUTH};

#define		NOS_SERVOS	(MOUTH + 1)

typedef enum  {SERVO, MOTOR} servo_type_te;

typedef enum {ABS_MOVE, ABS_MOVE_SYNC, SPEED_MOVE, SPEED_MOVE_SYNC, RUN_SYNC_MOVES, T_DELAY, STOP, STOP_ALL, ENABLE} servo_commands_te;
typedef enum {DISABLED, DORMANT, DELAY, MOVE, TIMED_MOVE} servo_states_te;

enum {SYS_INFO, SERVO_INFO, STEPPER_INFO};

struct servo_data_s {
	servo_states_te	state;
	bool			sync;
	servo_type_te	type;
	int32_t			angle;			// current value
	int32_t			angle_target;
	int32_t			speed_value;
	int32_t			init_angle;		// power-on state
	bool			flip;
	int32_t			angle_min, angle_max;
	uint32_t		pulse_offset;
    uint32_t        mS_per_degree;
	uint32_t		counter;
	float			gradient;
	float   		y_intercept;
	uint32_t		t_end;
};

//==============================================================================
//stepper motor interface (TMC2208)
//==============================================================================

#define     NOS_STEPPERS        1
#define     MAX_ST_STEP_CMDS   16
#define     ST_SEQUENCES        8

#define     MAX_STEPS           1000
#define     MIN_STEP_MOVE       4    // ignore very small stepper motor moves

#define     NOS_PROFILES        5
#define     NO_PROFILE          -1

#define     CALIBRATE_SPEED_DELAY   5       // number of mS between calibrate step pulses
 
typedef enum {CLOCKWISE = 0, ANTI_CLOCKWISE = 1} sm_direction;
enum {CLOCKWISE_COUNT_VALUE = +1, ANTI_CLOCKWISE_COUNT_VALUE = -1};

enum {OFF, ON};
enum {ASSERTED_LOW=0, ASSERTED_HIGH=1};

typedef enum {SM_REL_MOVE, SM_ABS_MOVE, SM_REL_MOVE_SYNC, SM_ABS_MOVE_SYNC, SM_CALIBRATE} stepper_commands_te;
    #define NOS_STEPPER_CMDS        (SM_CALIBRATE + 1)
typedef enum {SM_ACCEL, SM_COAST, SM_DECEL, SM_SKIP, SM_END , SM_DELAY} sm_command_type_et;

// Stepper motor run state machine states

typedef enum {
    STATE_SM_UNCALIBRATED, STATE_SM_DORMANT, STATE_SM_INIT, STATE_SM_RUNNING, STATE_SM_FAULT, STATE_SM_SYNC,
    STATE_SM_CALIB_S0, STATE_SM_CALIB_S1, STATE_SM_CALIB_S2, STATE_SM_CALIB_S3, STATE_SM_CALIB_S4,
    STATE_SM_CALIB_S5, STATE_SM_CALIB_S6, STATE_SM_CALIB_S7, STATE_SM_CALIB_S8, STATE_SM_CALIB_S9, 
    STATE_SM_CALIB_S10, STATE_SM_CALIB_S11,
} sm_profile_exec_state_te;


// Stepper motor data structure

struct stepper_data_s {
  // constant config data set at power-on time
    int32_t     steps_per_rev;
    int32_t     gearbox_ratio;
    int32_t     microstep_value;
    float       steps_per_degree;   //calculated at run time
    uint32_t    step_pin, direction_pin, R_limit_pin, L_limit_pin;
    sm_direction direction;
    bool        flip_direction;     // default is +ve for clockwise
    int32_t     init_step_position; // initial position from origin
  // set when motor is calibrated
    bool        calibrated;
    int32_t     max_step_count;
    int32_t     soft_left_limit, soft_right_limit;   // in angle for 0 centre
  // set per move
    int32_t     sm_profile;         // index of trapezoidal sm_profile
    int32_t     target_step_count;  // from command
    error_codes_te error;   
  // dynamic data changed as motor moves
    sm_profile_exec_state_te   state;
    int32_t     cmd_index;          // points to current command
    int32_t     cmd_step_cnt;       // number of steps at a fixed speed
    int32_t     coast_step_count;   // sm_profiles always have this set to 0
    int32_t     current_step_count; // from origin point
    int32_t     current_step_delay, current_step_delay_count; 
    int32_t     temp_count;
};

struct sm_step_cmd_s {      // single step motor command
    sm_command_type_et     sm_command_type;
    int32_t                sm_cmd_step_cnt;
    uint32_t               sm_delay;
};

struct sm_profile_s {      // single stepper motor seqence
    uint32_t    nos_sm_cmds;
    struct      sm_step_cmd_s  cmds[MAX_ST_STEP_CMDS];
};

//==============================================================================
// Neopixel subsystem
//==============================================================================

#define     NEOPIXEL_DOUT_PIN       GP26

#define     NOS_NEOPIXELS           10

#define     NEOPIXEL_DATA_RATE      800000      // 800KHz
#define     NEOPIXEL_BITS_PER_UNIT  24

#define     NEOPIXEL_PIO_UNIT       pio0
#define     NEOPIXEL_STATE_MACHINE  0

#define     NEOPIXEL_MAX_INTENSITY       25      // percent

typedef enum  {LED_NO_CHANGE, LED_OFF, LED_FLASH, LED_ON} neopixel_state_te;
typedef enum  { UP, DOWN, NONE} change_mode_et;
typedef enum  {N_WHITE, N_RED, N_ORANGE, N_YELLOW, N_GREEN, N_BLUE, N_INDIGO, N_VIOLET, N_BLACK} colours_et;
    #define NOS_NEOPIXEL_COLOURS   (N_BLACK + 1)
typedef enum  {N_OFF, N_ON, N_FLASH_OFF, N_FLASH_ON} NEOPIXEL_STATE_et;
typedef enum  {N_CMD_ON, N_CMD_OFF, N_CMD_FLASH} NEOPIXEL_CMD_et;

typedef enum {NP_SET_PIXEL_ON, NP_SET_PIXEL_OFF, NP_SET_PIXEL_FLASH, NP_BLANK_ALL} neopixel_commands_te;
    #define NOS_SERVO_CMDS     (NP_BLANK_ALL + 1) 

struct neopixel_data_s {
    NEOPIXEL_CMD_et      command;
    neopixel_state_te    state;
    colours_et           current_colour;
    uint32_t             current_intensity;     
    colours_et           on_colour;         // 3 8-bit GRB values
    uint32_t             on_intensity;      // percentage
    colours_et           off_colour;        // 3 8-bit GRB values
    uint32_t             off_intensity;     // percentage
    uint8_t              flash_on_time;     // units of 100mS
    int32_t              flash_on_counter;
    uint8_t              flash_off_time;    // units of 100mS
    int32_t              flash_off_counter;
    uint32_t             flash_counter;
    int8_t               dim_percent_change;     // +/- % rate
    uint8_t              dim_rate;               // units og 200mS
} ;

struct neopixel_colour_s {
    uint8_t   red;
    uint8_t   green;
    uint8_t   blue;
    uint32_t  GRB_value;
};

// struct neopixel_data_s {
//     struct {
//         NEOPIXEL_STATE_et   neopixel_state;         // ENABLED/DISABLED
//         NEOPIXEL_STATE_et   colour_rotation_state;  // ON/OFF
//         NEOPIXEL_STATE_et   flash_state;            // ON/OFF
//         NEOPIXEL_STATE_et   dim_state;
//         bool                monochrome;             // TRUE/FALSE
//     } flags;

//     uint8_t         current_intensity;
//     colours_et      current_colour;

//     uint8_t         flash_rate;
//     uint8_t         flash_counter;

//     int8_t          dim_percent_change;     // +/- % rate
//     uint8_t         dim_rate;               // units og 200mS
// } ;

//==============================================================================
// Freertos : task rates
//==============================================================================

#define     TASK_SERVO_CONTROL_FREQUENCY                 10  // Hz
#define     TASK_SERVO_CONTROL_FREQUENCY_TICK_COUNT      ((1000/TASK_SERVO_CONTROL_FREQUENCY) * portTICK_PERIOD_MS)


#define     TASK_SCAN_TOUCH_BUTTONS_FREQUENCY             5  // Hz
#define     TASK_SCAN_TOUCH_BUTTONS_FREQUENCY_TICK_COUNT      ((1000/TASK_SCAN_TOUCH_BUTTONS_FREQUENCY) * portTICK_PERIOD_MS)

#define     TASK_NEOPIXELS_FREQUENCY                     10  // Hz
#define     TASK_NEOPIXELS_TIME_UNIT                    (1000 / TASK_NEOPIXELS_FREQUENCY)
#define     TASK_NEOPIXELS_FREQUENCY_TICK_COUNT         ((1000/TASK_NEOPIXELS_FREQUENCY) * portTICK_PERIOD_MS)

//==============================================================================
// Set of 8 priority levels (set 8 in FreeRTOSconfig.h)
//==============================================================================

#define   TASK_PRIORITYIDLE             0
#define   TASK_PRIORITYLOW              1
#define   TASK_PRIORITYBELOWNORMAL      2
#define   TASK_PRIORITYNORMAL           3
#define   TASK_PRIORITYABOVENORMAL      4
#define   TASK_PRIORITYHIGH             5
#define   TASK_PRIORITYREALTIME         6
#define   TASK_PRIORITYERROR            7

//==============================================================================
// Print task information
//==============================================================================

#define     NOS_PRINT_STRING_BUFFERS   8
#define     MAX_PRINT_STRING_LENGTH     128

//==============================================================================
// Command string index values
//
// Common command indices

#define     PRIMARY_CMD_INDEX       0
#define     PORT_INDEX              1

// Stepper command indicies

#define     STEP_MOTOR_CMD_INDEX        2
#define     STEP_MOTOR_NO_INDEX         3
#define     STEP_MOTOR_ANGLE_INDEX      4
#define     STEP_MOTOR_PROFILE_INDEX    5

// servo command indicies

#define     SERVO_CMD_INDEX     2
#define     SERVO_NUMBER_INDEX  3
#define     SERVO_ANGLE_INDEX   4
#define     SERVO_SPEED_INDEX   5

// display command indicies

#define     DISPLAY_CMD_INDEX       2
#define     DISPLAY_FORM_INDEX      3   // for SET_FORM command
#define     DISPLAY_CONTRAST_INDEX  3   // for SET_CONTRAST command

// neopixel command indicies

#define     NEOPIXEL_CMD_INDEX                 2
#define     NEOPIXEL_NUMBER_INDEX              3
#define     NEOPIXEL_COLOUR_INDEX              4
#define     NEOPIXEL_FLASH_ON_COLOUR_INDEX     4
#define     NEOPIXEL_FLASH_ON_TIME_INDEX       5
#define     NEOPIXEL_FLASH_OFF_COLOUR_INDEX    6
#define     NEOPIXEL_FLASH_OFF_TIME_INDEX      7

//==============================================================================
// 

typedef enum {
    TASK_UART, TASK_RUN_CMD, TASK_SERVO_CONTROL, TASK_STEPPER_CONTROL,
    TASK_DISPLAY, TASK_SCAN_TOUCH_BUTTONS, TASK_WRITE_NEOPIXELS, TASK_BLINK,
} task_et;

#define     NOS_TASKS   (TASK_BLINK + 1)

//==============================================================================
/**
 * @brief Task data
 */
struct task_data_s {
    TaskHandle_t    task_handle;
    uint8_t         priority;
    StackType_t     *pxStackBase;
    configSTACK_DEPTH_TYPE  StackHighWaterMark;
    struct {
        uint32_t    last_exec_time;
        uint32_t    lowest_exec_time;
        uint32_t    highest_exec_time;
    };
};

#define UNDEFINED_PORT  -1

struct error_list_s {
    int32_t     error_code;
    char        *error_string;
};

struct token_list_s {
    char      *keyword;
    uint32_t  token;
};

struct min_max_s {
    int32_t     parameter_min;
    int32_t     parameter_max;
};

struct command_limits_s {
    struct min_max_s  p_limits[MAX_ARGC];
};

typedef enum  { ACK_NAK, NAK_REPORT , NAK_CMD , NAK_REPLY, ILLEGAL } display_reply_type_te;
typedef enum  { HOST_TO_DISPLAY, DISPLAY_TO_HOST } display_cmd_direction_te;

struct display_cmd_reply_data_s {
	display_cmd_direction_te  direction;
	int8_t					  length;
	display_reply_type_te	  reply_type;
} ;

enum {
    TOKENIZER_SERVO,
    TOKENIZER_STEPPER,
    TOKENIZER_SYNC,
    TOKENIZER_SET,
    TOKENIZER_GET,
    TOKENIZER_PING,
    TOKENIZER_TDELAY,
    TOKENIZER_DISPLAY,
    TOKENIZER_NEOPIXEL,
    TOKENIZER_ERROR,
};

#define NOS_COMMANDS   (TOKENIZER_ERROR + 1)

//==============================================================================
// Structure to hold button/form data

// #define GEN4_uLCD_MAX_NOS_FORMS                 8
// #define GEN4_uLCD_MAX_BUTTONS_PER_FORM          8
// #define GEN4_uLCD_MAX_NOS_SWITCHES_PER_FORM     64
// #define GEN4_uLCD_MAX_NOS_STRINGS_PER_FORM      8
// #define GEN4_uLCD_MAX_STRING_CHARS              32  

// #define     GEN4_uLCD_MAX_NOS_BUTTONS   64

// typedef struct  {
//     bool        enable;
//     uint8_t     object_type;
//     uint8_t     object_id;    // e.g. WINBUTTON0, WINBUTTON1, etc.
// 	int8_t	    button_value;
//     int32_t     time_high;      // High time in time sample units
// } touch_button_data_ts;

// typedef struct  {
//     object_state_te        enable;
//     uint8_t     object_type;
//     uint8_t     object_id;    // e.g. ISWITCHB0, ISWITCHB1, etc.
// 	int8_t	    switch_value;
// } touch_switch_data_ts;

// typedef struct {
//     object_state_te    enable;
//     char    string[GEN4_uLCD_MAX_STRING_CHARS + 1];  // +1 for null terminator
// } string_data_ts;

// typedef struct {
//     touch_button_data_ts    buttons[GEN4_uLCD_MAX_BUTTONS_PER_FORM];
//     touch_switch_data_ts    switches[GEN4_uLCD_MAX_NOS_SWITCHES_PER_FORM];
//     string_data_ts          strings[GEN4_uLCD_MAX_NOS_STRINGS_PER_FORM];
// } form_data_ts;

#endif /* __SYSTEM_H__ */   
