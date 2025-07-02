/**
 * @file Task_servo_control.c
 * @author Jim Herd
 * @brief Manage RC servo motors
 */
#include <stdio.h>
#include <stdlib.h>

#include "system.h"
#include "externs.h"
#include "uart_IO.h"
#include "sys_routines.h"
#include "PCA9685.h"

#include  "Pico_IO.h"

#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "FreeRTOS.h"

void Task_servo_control(void *p) {

TickType_t  xLastWakeTime;
BaseType_t  xWasDelayed;
uint32_t    start_time, end_time;
uint32_t    sample_count;
int32_t     angle, delta_angle, travel_time_count;

servo_states_te      RC_state;

    init_PCA9685_servo_IO();
    for (uint8_t i = 0; i < NOS_SERVOS ; i++) {
            servo_data[i].state = DORMANT ;
    }
    servo_data[MOUTH].state = DISABLED;

    set_servo_move(0, MOVE, 0, false);
    
    sample_count = 0;
    xLastWakeTime = xTaskGetTickCount ();
    FOREVER {
        xWasDelayed = xTaskDelayUntil( &xLastWakeTime, TASK_SERVO_CONTROL_FREQUENCY_TICK_COUNT );
        //START_PULSE;
        start_time = time_us_32();
        sample_count++;
        for (uint32_t i = 0; i < NOS_SERVOS; i++) {
            switch (servo_data[i].state) {
                case DISABLED :
                    PCA9685_set_zero(i);    
                    break;
                case DORMANT :
                    break;        // do nothing
                case DELAY :
                    servo_data[i].counter--;   // count down for delay
                    if (servo_data[i].counter == 0) {
                        servo_data[i].state = DORMANT;
                    }
                    break;
                case MOVE :
                    if (servo_data[i].sync == true) {
                        break;
                    }
                    PCA9685_set_servo(i, servo_data[i].angle);
                    delta_angle = abs(servo_data[i].angle - servo_data[i].angle_target);
                    travel_time_count = ((delta_angle * servo_data[i].mS_per_degree) / TASK_SERVO_CONTROL_FREQUENCY_TICK_COUNT) + 1;
                    servo_data[i].counter = travel_time_count; 
                    servo_data[i].state = DELAY;
                    break;
                case TIMED_MOVE :
                    if (servo_data[i].sync == true) {
                        break;
                    }
                    if (servo_data[i].counter == servo_data[i].t_end) {
                        servo_data[i].state = DORMANT;
                    } else {
                        angle = (int32_t)(servo_data[i].gradient * (float)servo_data[i].counter) + servo_data[i].y_intercept;
                        PCA9685_set_servo(i, angle);
                        servo_data[i].counter++;
                    }
                    break;
                default :
                    break;
            }
        }

        end_time = time_us_32();
        update_task_execution_time(TASK_SERVO_CONTROL, start_time, end_time);   
        //STOP_PULSE;
    }
}

