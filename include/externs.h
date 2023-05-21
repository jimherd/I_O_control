/**
 * @file    externs.h
 * @author  Jim Herd 
 * @brief   List of "extern" items
 */

#ifndef __EXTERNS_H__
#define __EXTERNS_H__

#include    "pico/stdlib.h"

#include    "system.h"

//==============================================================================
// FreeRTOS components

extern void Task_UART(void *p);
extern void Task_blink(void *p);
extern void Task_run_cmd(void *p);
extern void Task_servo_control(void *p);
extern void Task_stepper_control(void *p);

extern QueueHandle_t       queue_print_string_buffers;
extern QueueHandle_t       queue_free_buffers;

extern EventGroupHandle_t eventgroup_uart_IO;

//==============================================================================
// data structures

extern struct stepper_data_s        stepper_data[NOS_STEPPERS];
extern struct command_limits_s      cmd_limits[NOS_COMMANDS];
extern struct sm_profile_s              sequences[NOS_PROFILES];
extern char print_string_buffers[NOS_PRINT_STRING_BUFFERS][MAX_PRINT_STRING_LENGTH];
extern struct task_data_s           task_data[NOS_TASKS];
extern const uint8_t                char_type[256];
extern struct servo_data_s          servo_data[NOS_SERVOS];
extern struct token_list_s          commands[NOS_COMMANDS];

#endif  // __EXTERNS_H__