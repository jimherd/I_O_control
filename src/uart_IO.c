//==============================================================================
// buffered_uart.c     interrupt driven UART routines
//==============================================================================

#include    <stdio.h>
#include    <string.h>
#include    "pico/stdlib.h"
#include    "pico/binary_info.h"

#include    "hardware/gpio.h"
#include    "hardware/uart.h"
#include    "hardware/irq.h"

#include    "FreeRTOS.h"

#include    "system.h"
#include    "uart_IO.h"

#define     RING_BUFF_SIZE  255     // MUST be 2^n value

struct ring_buffer_s {
    uint32_t    in_pt;
    uint32_t    out_pt;
    uint32_t    count;
    char        buffer[RING_BUFF_SIZE];
};

struct ring_buffer_s  ring_buffer_in, ring_buffer_out;

//==============================================================================
// Deal with UART Tx/Rx interrupts
//==============================================================================

static void uart_interrupt_handler(void) {

   uint32_t data, ctrl = UART->mis;

    if(ctrl & (UART_UARTMIS_RXMIS_BITS | UART_UARTIMSC_RTIM_BITS)) {
        // while (!(UART->fr & UART_UARTFR_RXFE_BITS)) {
        //     data = UART->dr & 0xFF;                                     // Read input (use only 8 bits of data)
        //     if(!enqueue_realtime_command((char)data)) {
        //         uint_fast16_t next_head = BUFNEXT(rxbuf.head, rxbuf);   // Get next head pointer
        //         if(next_head == rxbuf.tail) {   // If buffer full
        //             rxbuf.overflow = true;      // flag overflow
        //         } else {
        //             rxbuf.data[rxbuf.head] = (char)data;                // Add data to buffer
        //             rxbuf.head = next_head;                             // and update pointer
        //         }
        //     }
        // }
    }

    // Interrupt if the TX FIFO is lower or equal to the empty TX FIFO threshold
    if(ctrl & UART_UARTMIS_TXMIS_BITS)
    {
        // uint_fast16_t tail = txbuf.tail;

        // // As long as the TX FIFO is not full or the buffer is not empty
        // while((!(UART->fr & UART_UARTFR_TXFF_BITS)) && (tail != txbuf.head)) {
        //     UART->dr = txbuf.data[tail];    // Put character in TX FIFO
        //     tail = BUFNEXT(tail, txbuf);    // and update tmp tail pointer
        // }
        // txbuf.tail = tail;                  //  Update tail pointer

        // if(txbuf.tail == txbuf.head)	    // Disable TX interrupt when the TX buffer is empty
        //     hw_clear_bits(&UART->imsc, UART_UARTIMSC_TXIM_BITS);
    }
}

//==============================================================================
// Uart routines
//==============================================================================
/**
 * @brief   Output a string to the UART buffer
 * 
 * @note    Backgroung interrupts will manage the character transfer
 * 
 * @param   str     string to be send - no '\n' required
 */
void uart_println(char *str) 
{
uint32_t    char_cnt, index;
char        *char_pt;

    char_cnt = strlen(str);
    char_pt = &ring_buffer_out.buffer[ring_buffer_out.out_pt];

    for (index = 0 ; index < char_cnt ; index++) {
        ring_buffer_out.buffer[ring_buffer_out.out_pt++] = str[index];
        ring_buffer_out.out_pt &= RING_BUFF_SIZE;  // do roll over
    }
    ring_buffer_out.buffer[ring_buffer_out.out_pt] = '\n';
    ring_buffer_out.out_pt &= RING_BUFF_SIZE;    // do roll over
}

/**
 * @brief   Task to manage printing to UART channel
 * 
 * @return  single character from input ring buffer
 */
char uart_getchar(void) {

}
//==============================================================================
// Task code
//==============================================================================
/**
 * @brief   Task to manage printing to UART channel
 * 
 * @param   p 
 * 
 * @note
 *          
 */
void Task_UART(void *p) {

    // init ring buffers

    ring_buffer_in.in_pt = 0;
    ring_buffer_in.out_pt = 0;
    ring_buffer_in.count = 0;

    ring_buffer_out.in_pt = 0;
    ring_buffer_out.out_pt = 0;
    ring_buffer_out.count = 0;

    FOREVER {
        
    }
}