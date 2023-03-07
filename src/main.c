/**
 * @file    main.c
 * @author  Jim Herd
 * @brief   Folder template for RP2040/FreeRTOS projects
 *          RP2040/FreeRTOS/TinyUSB/1306 LCD/DRV8833 H-bridge
 * Test
 */

#include "system.h"

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

// Hardware

const uint LED_PIN = PICO_DEFAULT_LED_PIN;
const uint LOG_PIN = GP2;
const uint BLINK_PIN = LED_PIN;

// FreeRTOS components handles

TaskHandle_t taskhndl_Task_blink_LED;
TaskHandle_t taskhndl_Task_run_cmd;
EventGroupHandle_t eventgroup_uart_IO;

// SemaphoreHandle_t semaphore_1;
// QueueHandle_t queue_1;
// EventGroupHandle_t eventgroup_1;

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

    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_puts(UART_ID, "Hello\n");

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

        xTaskCreate(Task_run_cmd,
                "Command_execution_task",
                configMINIMAL_STACK_SIZE,
                NULL,
                TASK_PRIORITYIDLE,
                &taskhndl_Task_run_cmd
    );

    eventgroup_uart_IO = xEventGroupCreate (); 

    vTaskStartScheduler();

    HANG;

    return 0;
}

