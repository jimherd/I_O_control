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

void uart_sys_init(void);
void uart_println(char *str);
char uart_getchar(void);
uint32_t uart_getline(char *string);

static void uart_interrupt_handler(void);