/**
 * @file    main.c
 * @author  Jim Herd
 * @brief   Folder template for RP2040/FreeRTOS projects
 *          RP2040/FreeRTOS/TinyUSB/1306 LCD/DRV8833 H-bridge
 */

#include "system.h"
#include "externs.h"
#include "sys_routines.h"
#include "uart_IO.h"

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/binary_info.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/i2c.h"

#include  "Pico_IO.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

//==============================================================================
// Global data
//==============================================================================
// set of print buffers

char print_string_buffers[NOS_PRINT_STRING_BUFFERS][MAX_PRINT_STRING_LENGTH];

struct task_data_s  task_data[NOS_TASKS];

// FreeRTOS components handles

TaskHandle_t        taskhndl_Task_uart;
TaskHandle_t        taskhndl_Task_blink;
TaskHandle_t        taskhndl_Task_run_cmd;
TaskHandle_t        taskhndl_Task_servo_control;
TaskHandle_t        taskhndl_Task_stepper_control;
TaskHandle_t        taskhndl_Task_display_control;

QueueHandle_t       queue_print_string_buffers;
QueueHandle_t       queue_free_buffers;

EventGroupHandle_t  eventgroup_uart_IO;

//==============================================================================
// System initiation
//==============================================================================

void init_system_data(void)
{

}

//==============================================================================
/**
 * @brief   Initialise datastore, some hardware, and FreeRTOS elements
 * 
 * @return int 
 */
int main() 
{
    stdio_init_all();

    gpio_init(LOG_PIN);
    gpio_set_dir(LOG_PIN, GPIO_OUT);
    gpio_pull_down(LOG_PIN);         // should default but just make sure


        // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    adc_init();

    init_system_data();

    xTaskCreate(Task_blink,
                "Blink_task",
                configMINIMAL_STACK_SIZE,
                NULL,
                TASK_PRIORITYIDLE,
                &taskhndl_Task_blink
    );

    xTaskCreate(Task_UART,
                "uart_task",
                configMINIMAL_STACK_SIZE,
                NULL,
                TASK_PRIORITYBELOWNORMAL,
                &taskhndl_Task_uart
    );

    xTaskCreate(Task_run_cmd,
                "Command_execution_task",
                configMINIMAL_STACK_SIZE,
                NULL,
                TASK_PRIORITYNORMAL,
                &taskhndl_Task_run_cmd
    );

    xTaskCreate(Task_servo_control,
                "RC_servo_control_task",
                configMINIMAL_STACK_SIZE,
                NULL,
                TASK_PRIORITYNORMAL,
                &taskhndl_Task_servo_control
    );

    xTaskCreate(Task_stepper_control,
                "Stepper_motor_control_task",
                512,     // configMINIMAL_STACK_SIZE,
                NULL, 
                TASK_PRIORITYNORMAL,
                &taskhndl_Task_stepper_control
    );

    xTaskCreate(Task_display_control,
                "4D_System_displaycontrol_task",
                512,     // configMINIMAL_STACK_SIZE,
                NULL,
                TASK_PRIORITYLOW,
                &taskhndl_Task_display_control
    );

    queue_print_string_buffers = xQueueCreate(NOS_PRINT_STRING_BUFFERS+1, sizeof(uint32_t));
    queue_free_buffers   = xQueueCreate(NOS_PRINT_STRING_BUFFERS+1, sizeof(uint32_t));
    
    prime_free_buffer_queue();

    eventgroup_uart_IO = xEventGroupCreate (); 

    vTaskStartScheduler();

    HANG;

    return 0;
}

