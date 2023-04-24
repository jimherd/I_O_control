//==============================================================================
// buffered_uart.c     interrupt driven UART routines
//==============================================================================

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
#include    "sys_routines.h"
#include    "uart_IO.h"
#include    "string_IO.h"
#include    "min_printf.h"

//==============================================================================
// System data
//==============================================================================

struct ring_buffer_s  ring_buffer_in, ring_buffer_out;

//==============================================================================
// Interrupt  handler : Deal with UART Tx/Rx interrupts
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
             if (ring_buffer_in.in_pt >= RING_BUFF_SIZE) {
                ring_buffer_in.in_pt = 0;   // rotate in pointer to the beginning
             }
             if (data == NEWLINE) {     // set event flag if line of data received
                xEventGroupSetBitsFromISR(eventgroup_uart_IO, (1 <<LINE_AVAILABLE), pdFALSE);
             }
        }
    }

    // Interrupt if the TX FIFO is lower or equal to the empty TX FIFO threshold
    // Transmit interrupt processing

    if(ctrl & UART_UARTMIS_TXMIS_BITS) {
        // // As long as the TX FIFO is not full or the buffer is not empty
        while((!(UART->fr & UART_UARTFR_TXFF_BITS)) && (ring_buffer_out.count != 0)) {
            UART->dr = ring_buffer_out.buffer[ring_buffer_out.out_pt++];    // Put character in TX FIFO
            ring_buffer_out.count--;
            if (ring_buffer_out.out_pt >= RING_BUFF_SIZE) {
                ring_buffer_out.out_pt = 0;   // rotate in pointer to the beginning
             }
            if(ring_buffer_out.count == 0)	{  // Disable TX interrupt when the TX buffer is empty
                hw_clear_bits(&UART->imsc, UART_UARTIMSC_TXIM_BITS);
            } else {
                hw_set_bits(&UART->imsc, UART_UARTIMSC_TXIM_BITS);
            }
        }
    }
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

uint32_t    xLastWakeTime, start_time, end_time;
uint32_t    string_index;

    uart_sys_init();

    xLastWakeTime = xTaskGetTickCount ();
    FOREVER {
        xQueueReceive(queue_print_string_buffers, &string_index,  portMAX_DELAY);
        start_time = time_us_32();
        uart_Write_string_buffer (string_index);
        end_time = time_us_32();
        update_task_execution_time(TASK_UART, start_time, end_time);
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

    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    uart_init(uart0, BAUD_RATE);
    uart_set_hw_flow(uart0, false, false);
    uart_set_format(uart0, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(uart0, true);

    irq_set_exclusive_handler(UART_IRQ, uart_interrupt_handler);
    irq_set_enabled(UART0_IRQ, true);

    hw_set_bits(&UART->imsc, UART_UARTIMSC_RXIM_BITS | UART_UARTIMSC_RTIM_BITS);
}

//==============================================================================
// UART input routines
//==============================================================================
/**
 * @brief Read a line of data from input ring buffer
 * 
 * @param string        pointer to string 
 * @return  int32_t     >0  number of characters in string
 *                      -1   error
 * @note
 *      Wait until line of data has been received in the input ring buffer.
 *      Task will not use CPU time during the wait period.
 *      No time-out on wait.
 * 
 *      Simple state machine to
 *          1. remove leading spaces
 *          2. ignore RETURN characters
 *          3. convert TAB character to a single SPACE
 *          4. replace NEWLINE with a STRING_NULL and exit
 */

typedef enum  {INIT, READ} line_read_states_te;

int32_t uart_readline(char *string)
{
uint32_t    i, ch_count, event_bits;
char        ch;
line_read_states_te  state;

    event_bits = xEventGroupWaitBits  ( 
                        eventgroup_uart_IO,
                        (1 << LINE_AVAILABLE),
                        pdTRUE,        //  clear flag
                        pdFALSE,        
                        portMAX_DELAY);
    state = INIT;
    ch_count = 0;
    for (i = 0; i < (MAX_STRING_LENGTH - 1) ; i++) {
        ch = uart_getchar();
        if (state == INIT) {
            if ((ch == SPACE) || (ch == TAB) || (ch == RETURN)) { // ignore char
                continue;    
            } else if (ch == NEWLINE) {      // string complete
                *string = STRING_NULL;
                return (ch_count + 1);
            } else {                         // load first character into buffer
                *string++ = ch;
                ch_count++;
                state = READ;
                continue;
            }
        }
        if (state == READ) {
            if (ch == NEWLINE) {
                *string = STRING_NULL;
                return (ch_count + 1);
            } else if (ch == TAB) {
                *string++ = SPACE;
                ch_count++;
            } else {
                *string++ = ch;
                ch_count++;
                continue;
            }
        }
    }
    *string = STRING_NULL;  // string has filled buffer
    return (ch_count + 1);
}

#define     DEBUG_REPLY     'D'
#define     CMD_REPLY       'C'
#define     MAX_SIZE_REPLY_STRING   128

struct string_buffer reply_buffer;

//==============================================================================
/**
 * @brief 
 * 
 * @param format 
 * @param ... 
 */
void print_string(const char *format, ...)
{
    init_string_buffer(&reply_buffer);
    va_list vargs;
    va_start(vargs, format);
    min_sprintf(&reply_buffer, format, vargs);
    va_end(vargs);
    uart_putstring(&reply_buffer.buffer[0]);
}

//==============================================================================
/**
 * @brief   read a character from the input circular buffer
 * @return * char 
 * @note    Access to buffer count is protected in a critical region.
 *          This stops possible corruption by a uart interrupt.
 */
char  uart_getchar(void)
{
char  ch;

    taskENTER_CRITICAL();
    if (ring_buffer_in.count != 0) {
        ring_buffer_in.count--;
    }
    ch = ring_buffer_in.buffer[ring_buffer_in.out_pt++];
    if (ring_buffer_in.out_pt >= RING_BUFF_SIZE) {
        ring_buffer_in.out_pt = 0;
    }
    taskEXIT_CRITICAL();
    return ch;
}

//==============================================================================
// UART output routines
//==============================================================================
/**
 * @brief Send string through buffered UART channel
 * 
 * @param string 
 * @note
 *      get index of free buffer
 *      copy string to this buffer
 *      send this buffer to UART subsystem
 *      return buffer to free list
 */
void uart_putstring(char *string)
{
uint32_t    buffer_index;

    xQueueReceive (queue_free_buffers, &buffer_index, portMAX_DELAY);
    char* in_pt = &string[0];
    char* out_pt = &print_string_buffers[buffer_index][0];
    uint32_t string_length = strlen(string);
    if (string_length >= MAX_PRINT_STRING_LENGTH) {
        string_length = MAX_PRINT_STRING_LENGTH - 1;
    }
    for (uint32_t i=0 ; i < string_length ; i++) {
        *out_pt++ = *in_pt++;
    }
    *out_pt = STRING_NULL;     // ensure string is null terminated
    uart_Write_string_buffer(buffer_index);
    xQueueSend(queue_free_buffers, &buffer_index, portMAX_DELAY);
}

//==============================================================================
/**
 * @brief output NULL terminated string to uart
 * 
 * @param   buffer_index    buffer number
 */
void uart_Write_string_buffer (uint32_t buffer_index)
{
    char    *ptr, ch;

    ptr = &print_string_buffers[buffer_index][0];
    while((ch = *ptr++) != '\0') {
        uart_putchar(ch);
    }
}

//==============================================================================
/**
 * @brief Output single character to uart or uart buffer
 * 
 * @param       c 
 * @return      true 
 * @return      false 
 * @note
 *      if buffer is empty AND harware FIFO is not full
 *          output character to hardware out register and exit
 *      else
 *          add character to buffer
 *          adjust in-pointer and character count
 *          enable uartTX interrupt
 *          exit
 */
static bool uart_putchar (const char ch)
{

    if ((ring_buffer_out.count == 0) && !(UART->fr & UART_UARTFR_TXFF_BITS)) {
        UART->dr = ch;  
        return true;
    } else {
        taskENTER_CRITICAL();
        ring_buffer_out.buffer[ring_buffer_out.in_pt++] = ch;
        if (ring_buffer_out.in_pt >= RING_BUFF_SIZE) {
            ring_buffer_out.in_pt = 0;
        }
        ring_buffer_out.count++;
        taskEXIT_CRITICAL();
        hw_set_bits(&UART->imsc, UART_UARTIMSC_TXIM_BITS);  // Enable transmit interrupt
        return true;
    }
}

//==============================================================================
/**
 * @brief prime free buffer queue with indices for all free buffers
 * 
 */
void prime_free_buffer_queue(void)
{
// struct string_buffer_s free_buffer_index;

    for (uint32_t count = 0; count < NOS_PRINT_STRING_BUFFERS; count++) {
        xQueueSend(queue_free_buffers, &count, portMAX_DELAY);
    }
    return;
}

//==============================================================================

static void uart_TxFlush (void)
{
    hw_clear_bits(&UART->imsc, UART_UARTIMSC_TXIM_BITS);
    ring_buffer_out.out_pt = 0;
    ring_buffer_out.out_pt = 0;
    ring_buffer_out.count = 0;
}

//==============================================================================

static void uart_RxFlush (void)
{
    while(!(UART->fr & UART_UARTFR_RXFE_BITS)) {
        UART->dr;
    }
    ring_buffer_in.out_pt = 0;
    ring_buffer_in.in_pt = 0;
    ring_buffer_out.count = 0;
}

