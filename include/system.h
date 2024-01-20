/**
 * @file    system.h
 * @author  Jim Herd 
 * @brief   General constants
 */

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include    "pico/stdlib.h"
#include    "Pico_IO.h"

#include    "FreeRTOS.h"
#include    "semphr.h"
#include    "event_groups.h"

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
#define     CPU_CLOCK_FREQUENCY         125000000   // 125MHz

//==============================================================================
// Useful times

#define     HALF_SECOND      (500/portTICK_PERIOD_MS)
#define     ONE_SECOND       (1000/portTICK_PERIOD_MS)
#define     TWO_SECONDS      (2000/portTICK_PERIOD_MS)

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
} error_codes_te;

//==============================================================================
// Serial comms port (UART)

#define UART0_TX_PIN     GP0
#define UART0_RX_PIN     GP1

#define UART0_ID uart0
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
#define     MAX_GEN4_uLCD_WRITE_STR_SIZE    70

#define     MAX_COMMAND_LENGTH      100

#define     MAX_ARGC  8

enum modes_e {MODE_U, MODE_I, MODE_R, MODE_S} ;  // defines modes as scan progresses

enum {LETTER, NUMBER, DOT, PLUSMINUS, NULTERM, END, SEPARATOR, OTHER};

enum {BASE_10 = 10, BASE_16 = 16};
enum {UPPER_CASE, LOWER_CASE};

//==============================================================================
// Serial display port (UART) - 4D System display

#define UART1_TX_PIN     GP4
#define UART1_RX_PIN     GP5

#define UART1_ID uart1
#define UART1_BAUD_RATE   115200

#define DISPLAY_RESET_PIN  GP3
#define DISPLAY_WAIT_US    25

typedef enum {SET_FORM, GET_FORM, SET_CONTRAST} display_commands_te;

#define     NOS_FORMS   3

//==============================================================================
//I2C port

#define I2C_PORT    i2c0
#define I2C_SDA     GP8
#define I2C_SCL     GP9

//==============================================================================
// log and blink pins

#define LED_PIN     PICO_DEFAULT_LED_PIN
#define LOG_PIN     GP2
#define BLINK_PIN   LED_PIN

//==============================================================================
//servo motor interface (PCA9685A)

#define     PCA9685_address     0x40

#define		PCA9685_servo_frequency		 50  // hertz
#define		PCA9685_50Hz_PRE_SCALER		138  // TUNED : calc = 123
  // refer to datasheet for calculation

#define		SERVO_TRIM_MIN		110
#define     SERVO_TRIM_MAX		590

#define		MID_POINT_COUNT		307
#define		COUNT_1mS			205
#define		MAX_ANGLE			 90

enum {L_EYE_LR, L_EYE_UD, L_EYE_LID, L_EYE_BROW, R_EYE_LR, R_EYE_UD, R_EYE_LID, R_EYE_BROW, MOUTH};

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
enum {ASSERTED_LOW, ASSERTED_HIGH};

typedef enum {SM_REL_MOVE, SM_ABS_MOVE, SM_REL_MOVE_SYNC, SM_ABS_MOVE_SYNC, SM_CALIBRATE} stepper_commands_te;
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
// Freertos

#define     TASK_SERVO_CONTROL_FREQUENCY                 10  // Hz
#define     TASK_SERVO_CONTROL_FREQUENCY_TICK_COUNT      ((1000/TASK_SERVO_CONTROL_FREQUENCY) * portTICK_PERIOD_MS)


#define     TASK_SCAN_TOUCH_BUTTONS_FREQUENCY            10  // Hz
#define     TASK_SCAN_TOUCH_BUTTONS_FREQUENCY_TICK_COUNT      ((1000/TASK_SCAN_TOUCH_BUTTONS_FREQUENCY) * portTICK_PERIOD_MS)
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

#define     NOS_PRINT_STRING_BUFFERS    8
#define     MAX_PRINT_STRING_LENGTH     128

//==============================================================================
// Command string index values
//
// Common

#define     PRIMARY_CMD_INDEX       0
#define     PORT_INDEX              1

// Stepper command

#define     STEP_MOTOR_CMD_INDEX        2
#define     STEP_MOTOR_NO_INDEX         3
#define     STEP_MOTOR_ANGLE_INDEX      4
#define     STEP_MOTOR_PROFILE_INDEX    5

// servo command

#define     SERVO_CMD_INDEX     2
#define     SERVO_NUMBER_INDEX  3
#define     SERVO_ANGLE_INDEX   4
#define     SERVO_SPEED_INDEX   5

// display command

#define     DISPLAY_CMD_INDEX       2
#define     DISPLAY_FORM_INDEX      3   // for SET_FORM command
#define     DISPLAY_CONTRAST_INDEX  3   // for SET_CONTRAST command

//==============================================================================
// 

typedef enum {
    TASK_UART, TASK_RUN_CMD, TASK_SERVO_CONTROL, TASK_STEPPER_CONTROL, TASK_BLINK,
    TASK_DISPLAY, TASK_SCAN_TOUCH_BUTTONS
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
    TOKENIZER_ERROR,
};

#define NOS_COMMANDS   (TOKENIZER_ERROR + 1)

//==============================================================================
// Structure to hold button/form data

#define     MAX_NOS_FORMS           8
#define     MAX_NOS_FORM_BUTTONS    8

struct touch_button_data_s {
	uint32_t	button_index;
	uint32_t	button_data;
    int32_t     time_high;
} ;

struct form_data_s {
	bool        form_focus;
    uint32_t    nos_buttons;
	struct touch_button_data_s	    button[MAX_NOS_FORM_BUTTONS];
}  ;


#endif /* __SYSTEM_H__ */
