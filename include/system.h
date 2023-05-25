/**
 * @file    system.h
 * @author  Jim Herd 
 * @brief   General constants
 */

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include    "PCA9685.h"

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
    OK                      = 0,
    LETTER_ERROR            = -100,
    DOT_ERROR               = -101,
    PLUSMINUS_ERROR         = -102,
    BAD_COMMAND             = -103,
    BAD_PORT_NUMBER         = -104,
    BAD_NOS_PARAMETERS      = -105,
    BAD_BASE_PARAMETER      = -106,
    PARAMETER_OUTWITH_LIMITS = -107,
    BAD_SERVO_COMMAND       = -108,
    STEPPER_CALIBRATE_FAIL  = -109,
    BAD_STEPPER_COMMAND     = -110,
    BAD_STEP_VALUE          = -111,
    MOVE_ON_UNCALIBRATED_MOTOR = -112,
    EXISTING_FAULT_WITH_MOTOR  = -113,
    SM_MOVE_TOO_SMALL       = -114,
} error_codes_te;

//==============================================================================
// Serial comms port (UART)

#define UART_TX_PIN     GP0
#define UART_RX_PIN     GP1

#define UART_ID uart0
#define BAUD_RATE 115200

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

#define     MAX_COMMAND_LENGTH      100

#define     MAX_ARGC  8

enum modes_e {MODE_U, MODE_I, MODE_R, MODE_S} ;  // defines modes as scan progresses

enum {LETTER, NUMBER, DOT, PLUSMINUS, NULTERM, END, SEPARATOR, OTHER};

enum {BASE_10 = 10, BASE_16 = 16};
enum {UPPER_CASE, LOWER_CASE};

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
//stepper motor interface (A4988)

#define     NOS_STEPPERS        1
#define     MAX_ST_STEP_CMDS   16
#define     ST_SEQUENCES        8

#define     MAX_STEPS           500
#define     MIN_STEP_MOVE       4    // ignore very small stepper motor moves

#define     A4988_STEP          GP10
#define     A4988_DIRECTION     GP11
#define     LIMIT_SWITCH_1      GP12
#define     LIMIT_SWITCH_2      GP13

#define     NOS_PROFILES        5
#define     NO_PROFILE          -1

typedef enum {ANTI_CLOCKWISE=0,CLOCKWISE} sm_direction;

enum {OFF, ON};

typedef enum {SM_REL_MOVE, SM_ABS_MOVE, SM_REL_MOVE_SYNC, SM_ABS_MOVE_SYNC, SM_CALIBRATE} stepper_commands_te;
typedef enum {M_UNCALIBRATED, M_DORMANT, M_INIT, M_RUNNING, M_FAULT} sm_profile_exec_state_te;
typedef enum {SM_ACCEL, SM_COAST, SM_DECEL, SM_SKIP, SM_END} sm_profile_state_et;

struct stepper_data_s {
  // config data
    uint32_t    step_pin, direction_pin;
    sm_direction direction;
    bool        flip_direction;     // default is +ve for clockwise
    int32_t     init_step_count;    // initial position from origin
    int32_t     max_step_travel;
  // dynamic data
    sm_profile_exec_state_te   state;
    int32_t     sm_profile;             // index of trapezoidal sm_profile
    int32_t     cmd_index;          // points to current command
    int32_t     cmd_step_cnt;       // number of steps at a fixed speed
    int32_t     coast_step_count;   // sm_profiles always have this set to 0

    int32_t     current_step_count; // from origin point
    int32_t     target_step_count;  // from command
    int32_t     current_step_delay, current_step_delay_count;
    error_codes_te error;     
};

struct sm_step_cmd_s {      // single step motor command
    sm_profile_state_et    sm_profile_state;
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
// Macros
//==============================================================================

#define     START_PULSE         gpio_put(LOG_PIN, 1)
#define     STOP_PULSE          gpio_put(LOG_PIN, 0)

#define     FOREVER     for(;;)
#define     HANG        for(;;)

#define     ATTRIBUTE_PACKED     __attribute__ ((__packed__))

//==============================================================================
// Print task information
//==============================================================================

#define     NOS_PRINT_STRING_BUFFERS    8
#define     MAX_PRINT_STRING_LENGTH     128

//==============================================================================
// queue element for serial print facility
//
// Queue to print task  holds list of used indexes pointing to string to be printed
// Queue from print task holds list of indexes pointing to free string buffers



// Format of queues to/from print task

// struct string_buffer_s {
//     uint32_t    buffer_index;
// };

//==============================================================================
// 

typedef enum {
    TASK_UART, TASK_RUN_CMD, TASK_SERVO_CONTROL, TASK_STEPPER_CONTROL, TASK_BLINK
} task_et;

#define     NOS_TASKS   (TASK_BLINK + 1)

//==============================================================================
/**
 * @brief Task data
 */
struct task_data_s {
    TaskHandle_t            task_handle;
    uint8_t                 priority;
    StackType_t             *pxStackBase;
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

enum {
    TOKENIZER_SERVO,
    TOKENIZER_STEPPER,
    TOKENIZER_SYNC,
    TOKENIZER_CONFIG,
    TOKENIZER_INFO,
    TOKENIZER_PING,
    TOKENIZER_TDELAY,
    TOKENIZER_ERROR,
};

#define NOS_COMMANDS   (TOKENIZER_ERROR + 1)

#endif /* __SYSTEM_H__ */

// archive : delete later
// // FreeRTOS components

// extern void Task_UART(void *p);
// extern void Task_blink(void *p);
// extern void Task_run_cmd(void *p);
// extern void Task_servo_control(void *p);
// extern void Task_stepper_control(void *p);
// extern QueueHandle_t       queue_print_string_buffers;
// extern QueueHandle_t       queue_free_buffers;
// extern EventGroupHandle_t eventgroup_uart_IO;

// data structures
// extern char print_string_buffers[NOS_PRINT_STRING_BUFFERS][MAX_PRINT_STRING_LENGTH];
// extern struct task_data_s  task_data[NOS_TASKS];
// extern const uint8_t char_type[256];
// extern struct servo_data_s     servo_data[NOS_SERVOS];
// extern struct token_list_s commands[NOS_COMMANDS];