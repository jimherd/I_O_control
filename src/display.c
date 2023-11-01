/**
 * @file display.c
 * @author Jim Herd
 * @brief Control 4D Systems touch screen display
 */

#include    <stdio.h>
#include    <string.h>
#include    <stdarg.h>


#include    "pico/stdlib.h"
#include    "pico/binary_info.h"

#include    "hardware/gpio.h"
#include    "hardware/uart.h"
#include    "hardware/irq.h"

#include    "FreeRTOS.h"
#include    "event_groups.h"
#include    "timers.h"
#include    "queue.h"

#include    "system.h"
#include    "externs.h"
#include    "sys_routines.h"
#include    "uart_IO.h"
#include    "string_IO.h"
#include    "min_printf.h"

void uart1_sys_init(void)
{
    // ring_buffer_in.in_pt   = 0;
    // ring_buffer_in.out_pt  = 0;
    // ring_buffer_in.count   = 0;
    // ring_buffer_out.in_pt  = 0;
    // ring_buffer_out.out_pt = 0;
    // ring_buffer_out.count  = 0;

    gpio_set_function(UART1_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART1_RX_PIN, GPIO_FUNC_UART);

    uart_init(uart0, UART1_BAUD_RATE);
    uart_set_hw_flow(uart1, false, false);
    uart_set_format(uart1, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(uart1, true);

    // irq_set_exclusive_handler(UART_IRQ, uart_interrupt_handler);
    // irq_set_enabled(UART0_IRQ, true);

    // hw_set_bits(&UART->imsc, UART_UARTIMSC_RXIM_BITS | UART_UARTIMSC_RTIM_BITS);

    gpio_init(DISPLAY_RESET_PIN);
    gpio_set_dir(DISPLAY_RESET_PIN, GPIO_IN);
    gpio_put(DISPLAY_RESET_PIN, 0);
}

/**
 * @brief generate a reset pulse for the display
 * 
 * @return * void 
 */
void reset_4D_display(void)
{
    gpio_put(DISPLAY_RESET_PIN, 0);
    gpio_set_dir(DISPLAY_RESET_PIN, GPIO_OUT);
    busy_wait_us(DISPLAY_WAIT_US);
    gpio_set_dir(DISPLAY_RESET_PIN, GPIO_IN);
}
