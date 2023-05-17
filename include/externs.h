/**
 * @file    externs.h
 * @author  Jim Herd 
 * @brief   General constants
 */

#ifndef __EXTERNS_H__
#define __EXTERNS_H__

#include    "system.h"

extern struct stepper_data_s     stepper_data[NOS_STEPPERS];
extern struct command_limits_s    cmd_limits[NOS_COMMANDS];

#endif