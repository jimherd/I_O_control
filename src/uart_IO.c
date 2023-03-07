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

#define     RING_BUFF_SIZE  256     // MUST be 2^n value

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

// Receive interrupt processing

    if(ctrl & (UART_UARTMIS_RXMIS_BITS | UART_UARTIMSC_RTIM_BITS)) {
        while (!(UART->fr & UART_UARTFR_RXFE_BITS)) {
             data = UART->dr & 0xFF;                       // Read input (use only 8 bits of data)
             if (data == RETURN) {
                continue;           // ignore RETURN characters
             }
             if (data == TAB){
                data = SPACE;       // replace TABs with SPACEs
             }
             ring_buffer_in.buffer[ring_buffer_in.in_pt++] = (char)data;  // store data
             ring_buffer_in.count++;
             if (ring_buffer_in.in_pt > RING_BUFF_SIZE) {
                ring_buffer_in.in_pt = 0;   // rotate in pointer to the beginning
             }
             if (data == NEWLINE) {     // send event flag if line of data received
                xEventGroupSetBitsFromISR(eventgroup_uart_IO, LINE_AVAILABLE, pdFALSE);
             }
        }
    }

    // Interrupt if the TX FIFO is lower or equal to the empty TX FIFO threshold

    // Transmit interrupt processing

    if(ctrl & UART_UARTMIS_TXMIS_BITS) {
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
// uart routines
//==============================================================================
/**
 * @brief Initialise uart subsystem and associated interrupts
 */
void uart_sys_init(void)
{
    ring_buffer_in.in_pt   = 0;
    ring_buffer_in.out_pt  = 0;
    ring_buffer_in.count   = 0;
    ring_buffer_out.in_pt  = 0;
    ring_buffer_out.out_pt = 0;
    ring_buffer_out.count  = 0;

    gpio_set_function(UART_TX_PIN, GP0);
    gpio_set_function(UART_RX_PIN, GP1);

    uart_init(uart0, BAUD_RATE);
    uart_set_hw_flow(uart0, false, false);
    uart_set_format(uart0, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(uart0, true);

    irq_set_exclusive_handler(UART_IRQ, uart_interrupt_handler);
    irq_set_enabled(UART1_IRQ, true);

    hw_set_bits(&UART->imsc, UART_UARTIMSC_RXIM_BITS | UART_UARTIMSC_RTIM_BITS);
}

/**
 * @brief   read a character from the input circular buffer
 * @return * char 
 * @note    Access to buffer count is protected in a critical region.
 *          This stops possible corruption by a uart interrupt.
 */
char  uart_getchar(void)
{
    taskENTER_CRITICAL();
    if (ring_buffer_in.count != 0) {
        ring_buffer_in.count--;
    }
    taskEXIT_CRITICAL();
    return ring_buffer_in.buffer[ring_buffer_in.out_pt++];
}

/**
 * @brief Read a line of data from input ring buffer
 * 
 * @param string        pointer to string 
 * @return  uint32_t   number of characters in string
 * 
 * @note
 *      Wait until line of data has been received in the input ring buffer.
 */
uint32_t uart_readline(char *string)
{
uint32_t    i, ch_count, event_bits;
char        ch;

    event_bits = xEventGroupWaitBits  ( 
                        eventgroup_uart_IO,
                        (1 << LINE_AVAILABLE),
                        pdTRUE,        //  clear flag
                        pdFALSE,        
                        portMAX_DELAY);
    ch_count = 0;
    for (i = 0; i < (MAX_STRING_LENGTH - 1) ; i++) {
        ch = uart_getchar();
        if (ch == NEWLINE) {
            break;
        }
        *string++ = ch;
        ch_count++;
    }
    *string = STRING_NULL;
    return (ch_count + 1);
}

/**
 * @brief   Output a string to the UART buffer
 * @note    Backgroung interrupts will manage the character transfer
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


//==============================================================================
// Task code
//==============================================================================
/**
 * @brief   Task to manage printing to UART channel
 * @note
 *          
 */
void Task_UART(void *p) {

    uart_sys_init();

    FOREVER {
        
    }
}