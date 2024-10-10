/**
 * @file    externs.h
 * @author  Jim Herd 
 * @brief   List of "extern" items
 */

#ifndef __EXTERNS_H__
#define __EXTERNS_H__

#include    "pico/stdlib.h"

#include    "system.h"
#include    "gen4_uLCD.h"

//==============================================================================
// FreeRTOS components

extern void Task_UART(void *p);
extern void Task_blink(void *p);
extern void Task_run_cmd(void *p);
extern void Task_servo_control(void *p);
extern void Task_stepper_control(void *p);
extern void Task_display_control(void *p);
void Task_scan_touch_buttons(void *p);
extern void Task_write_neopixels(void *p);

extern QueueHandle_t       queue_print_string_buffers;
extern QueueHandle_t       queue_free_buffers;

extern EventGroupHandle_t eventgroup_uart_IO;

extern SemaphoreHandle_t   gen4_uLCD_MUTEX_access;
extern SemaphoreHandle_t   semaphore_neopixel_data;

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
extern struct display_cmd_reply_data_s    display_cmd_info[NOS_GEN4_uLCD_CMDS];
extern touch_button_data_ts   button_data[GEN4_uLCD_MAX_NOS_BUTTONS];
extern struct neopixel_data_s  neopixel_data[NOS_NEOPIXELS];

#endif  // __EXTERNS_H__