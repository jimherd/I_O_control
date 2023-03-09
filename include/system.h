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
// Serial comms port (UART)

#define UART_TX_PIN GP0
#define UART_RX_PIN GP1

#define UART_ID uart0
#define BAUD_RATE 115200

#define LINE_AVAILABLE  0   // bit in eventgroup_uart_IO

#define     RETURN      '\r'
#define     NEWLINE     '\n'
#define     TAB         '\t'
#define     SPACE       ' '
#define     STRING_NULL '\0'

#define     MAX_STRING_LENGTH  80

#define     MAX_COMMAND_LENGTH  100

#define     MAX_COMMAND_PARAMETERS   16

enum modes_e {MODE_U, MODE_I, MODE_R, MODE_S} ;  // defines modes as scan progresses

//==============================================================================
//I2C port

#define I2C_PORT    i2c0
#define I2C_SDA     GP8
#define I2C_SCL     GP9

//==============================================================================
// Freertos

typedef enum TASKS {
    TASK_BLINK
} task_t;

#define     NOS_TASKS   (TASK_BLINK + 1)

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
#define     STRING_LENGTH        128

//==============================================================================
// queue element for serial print facility
//
// Queue to print task  holds list of used indexes pointing to string to be printed
// Queue from print task holds list of indexes pointing to free string buffers



// Format of queues to/from print task

struct string_buffer_s {
    uint32_t    buffer_index;
};

//==============================================================================
// 

enum error_codes_e {
    OK,
    LETTER_ERROR    = -100,
    DOT_ERROR       = -101,
    PLUSMINUS_ERROR = -102,
    BAD_COMMAND     = -103,
    BAD_PORT_NUMBER = -104
};

//==============================================================================
// Extern references
//==============================================================================

//==============================================Error codesdefinitions of system data structures
//==============================================================================

// Hardware

extern const uint LED_PIN;
extern const uint LOG_PIN;
extern const uint BLINK_PIN;

// FreeRTOS components

extern void Task_UART(void *p);
extern void Task_blink_LED(void *p);
extern void Task_run_cmd(void *p);

extern QueueHandle_t       queue_print_string_buffers;
extern QueueHandle_t       queue_free_buffers;

extern EventGroupHandle_t eventgroup_uart_IO;

// extern  TaskHandle_t taskhndl_Task_blink_LED;
// extern SemaphoreHandle_t semaphore_1
// extern QueueHandle_t queue_1; 
// extern EventGroupHandle_t eventgroup_1;      // event groups

#endif /* __SYSTEM_H__ */