/**
 * @file Task_run_cmd.c
 * @author Jim Herd
 * @brief Read and execute command strings
 */
#include <stdio.h>
#include <stdlib.h>

#include "system.h"
#include "uart_IO.h"

#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "FreeRTOS.h"

char    command[MAX_STRING_LENGTH];

void Task_run_cmd(void *p) {

int32_t     char_count;

    FOREVER {
        char_count = uart_readline(command);
        printf("test\n");
 
    }
}
