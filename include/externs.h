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

//==============================================================================
// data structures

extern struct stepper_data_s        stepper_data[NOS_STEPPERS];
extern struct command_limits_s      cmd_limits[NOS_COMMANDS];
extern struct sm_seq_s              sequences[NOS_PROFILES];

#endif