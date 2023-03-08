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

//void Task_UART(void *p);

void uart_sys_init(void);
void uart_println(char *str);
char uart_getchar(void);
int32_t uart_readline(char *string);

static void uart_interrupt_handler(void);