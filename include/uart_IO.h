//==============================================================================
// Task_UART.h     
//==============================================================================

#ifndef UART_PORT
    #define UART_PORT uart0
    #define UART ((uart_hw_t *)UART_PORT)
    #define UART_IRQ UART0_IRQ
#endif

//==============================================================================
// Prototypes
//==============================================================================

void uart_println(char *str);
char uart_getchar(void);

static void uart_interrupt_handler(void);