/**
 * @file    main.c
 * @author  Jim Herd
 * @brief   Folder template for RP2040/FreeRTOS projects
 *          RP2040/FreeRTOS/TinyUSB/1306 LCD/DRV8833 H-bridge
 * Test
 */

#include "system.h"
#include "sys_routines.h"

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

char print_string_buffers[NOS_PRINT_STRING_BUFFERS][STRING_LENGTH];

// Hardware

const uint LED_PIN = PICO_DEFAULT_LED_PIN;
const uint LOG_PIN = GP2;
const uint BLINK_PIN = LED_PIN;

// FreeRTOS components handles

TaskHandle_t        taskhndl_Task_uart;
TaskHandle_t        taskhndl_Task_blink_LED;
TaskHandle_t        taskhndl_Task_run_cmd;

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

    
    // gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    // gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    // uart_init(UART_ID, BAUD_RATE);
    // uart_puts(UART_ID, "Hello\n");

        // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    adc_init();

    init_system_data();

    xTaskCreate(Task_blink_LED,
                "Blink_task",
                configMINIMAL_STACK_SIZE,
                NULL,
                TASK_PRIORITYIDLE,
                &taskhndl_Task_blink_LED
    );

    xTaskCreate(Task_UART,
                "uart_task",
                configMINIMAL_STACK_SIZE,
                NULL,
                TASK_PRIORITYIDLE,
                &taskhndl_Task_uart
    );

    xTaskCreate(Task_run_cmd,
                "Command_execution_task",
                configMINIMAL_STACK_SIZE,
                NULL,
                TASK_PRIORITYIDLE,
                &taskhndl_Task_run_cmd
    );

    queue_print_string_buffers = xQueueCreate(NOS_PRINT_STRING_BUFFERS+1, sizeof(struct string_buffer_s));
    queue_free_buffers   = xQueueCreate(NOS_PRINT_STRING_BUFFERS+1, sizeof(struct string_buffer_s));
    prime_free_buffer_queue();

    eventgroup_uart_IO = xEventGroupCreate (); 

    vTaskStartScheduler();

    HANG;

    return 0;
}

