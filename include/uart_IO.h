/**
 * @file uart_IO.h
 * @author Jim Herd (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-03-09
 */

#ifndef __UART_IO_H__
#define __UART_IO_H__

#ifndef UART_PORT
    #define UART_PORT   uart0
    #define UART        ((uart_hw_t *)UART_PORT)
    #define UART_IRQ    UART0_IRQ
#endif

#define     RING_BUFF_SIZE  256 

//==============================================================================
// Structures
//==============================================================================

struct ring_buffer_s {
    uint32_t    in_pt;
    uint32_t    out_pt;
    uint32_t    count;
    char        buffer[RING_BUFF_SIZE];
};

//==============================================================================
// Function prototypes
//==============================================================================

static void uart_interrupt_handler(void);

void uart0_sys_init(void);
void uart_println(char *str);
char uart_getchar(void);
int32_t uart_readline(char *string);
void uart_putstring(char *string);
void uart_Write_string_buffer(uint32_t string_index);
static bool uart_putchar(const char ch);
void prime_free_buffer_queue(void);

void print_string(const char *format, ...);

#endif    /*   __UART_IO_H__    */