/**
 * @file PCA9685.c
 * @author Jim Herd
 * @brief 
 * @date 2023-03-14
 * @note
 *          Routines to use PCA9685 with 50Hz servos. Not general 
 *          purpose routines. 
 */

#include  "stdlib.h"

#include  "system.h"
#include  "PCA9685.h"

#include "hardware/i2c.h"

//==============================================================================
// servo data with initial values

struct servo_data_s     servo_data[NOS_SERVOS] = {
    {true, false, SERVO, 0, 0, 0, 45, false, -45, +45,  0},
    {true, false, SERVO, 0, 0, 0, 45, false, -45, +45, 10},
    {true, false, SERVO, 0, 0, 0, 45, false, -45, +45, 20},
    {true, false, SERVO, 0, 0, 0, 45, false, -45, +45, 30},
    {true, false, SERVO, 0, 0, 0, 45, false, -45, +45, 40},
    {true, false, SERVO, 0, 0, 0, 45, false, -45, +45, 50},
    {true, false, MOTOR, 0, 0, 0, 45, false, -45, +45, 60},
};
uPCA9685_REG__MODE1     PCA9685_reg_mode1;

/**
 * @brief write byte to PCA9685
 * 
 * @param reg_number    PCA9685 register number
 * @param data_byte     8-bit value to be written to register
 */
void write_PCA9685_register(uint8_t reg_number, uint8_t data_byte)
{
    uint8_t i2c_write_packet[2];

    i2c_write_packet[0] = reg_number;
    i2c_write_packet[1] = data_byte;

    i2c_write_blocking(I2C_PORT, PCA9685_address, i2c_write_packet, 2, false);
    return;
}

/**
 * @brief   read byte from PCA9685
 * 
 * @param reg_number 
 * @return uint8_t 
 */
uint8_t read_PCA9685_register(uint8_t reg_number)
{
uint8_t  data;

    i2c_write_blocking(I2C_PORT, PCA9685_address, &reg_number, 1, false);
    i2c_read_blocking(I2C_PORT, PCA9685_address, &data, 1, false);

    return  data;
}

/**
 * @brief Initialise PCA9685 to drive RC servos
 * 
 */
void init_PCA9685_servo_IO(void)
{
uint8_t  data;

    PCA9685_set_sleep(true);
    PCA9685_set_servo_freq();
    PCA9685_set_auto_increment(true);
    PCA9685_set_sleep(false);
    return;
}

/**
 * @brief set PCA9685 into its sleep mode
 */
void inline PCA9685_set_sleep(bool mode) 
{
uint8_t PCA9685_mode1_data;

    PCA9685_mode1_data = read_PCA9685_register(PCA9685__MODE1);
    if (mode == true) {             // put to sleep
        PCA9685_mode1_data |= MODE_SLEEP_MASK;
        write_PCA9685_register(PCA9685__MODE1, PCA9685_mode1_data);
        vTaskDelay(5);
    } else {                        // wakeup
        PCA9685_mode1_data &= ~MODE_SLEEP_MASK;
        write_PCA9685_register(PCA9685__MODE1, PCA9685_mode1_data);
    }
}
/**
 * @brief reset PCA968a device
 */
void inline PCA9685_reset(void)
{
    write_PCA9685_register(PCA9685__MODE1, PCA9685_MODE1_RESTART);
}

/**
 * @brief enable/disable device autoincrement mode
 * 
 * @param mode 
 */
void  PCA9685_set_auto_increment(bool mode)
{
    uint8_t PCA9685_mode1_data;

    PCA9685_mode1_data = read_PCA9685_register(PCA9685__MODE1);
    if (mode == true) {             // put to sleep
        PCA9685_mode1_data |= PCA9685_MODE1_AUTO_INC;
        write_PCA9685_register(PCA9685__MODE1, PCA9685_mode1_data);
        vTaskDelay(5);
    } else {                        // wakeup
        PCA9685_mode1_data &= ~PCA9685_MODE1_AUTO_INC;
        write_PCA9685_register(PCA9685__MODE1, PCA9685_mode1_data);
    }
}

/**
 * @brief Set frequency to 50Hz for use with servos
 */
void PCA9685_set_servo_freq(void){

    PCA9685_reset();
    PCA9685_set_sleep(true);
    write_PCA9685_register(PCA9685__PRE_SCALE, PCA9685_50Hz_PRE_SCALER);
    PCA9685_set_sleep(false);
}
/**
 * @brief 
 * 
 * @param servo_no      0->15
 * @param angle         -90 -> +90
 * @note
 * 
 * Angles are specified as + or - values from a centre point.  This 
 * mid-point is a fixed datum no matter the angular range 
 * e.g. +/-90, +/-45 etc.
 * servo freq = 50Hz , period = 20mS  => 1ms = 4096/20 = 204.8 counts
 * Midpoint is at 1.5mS = 307.2 counts
 * +angle  =>  add to 1.5mS pulse
 * -angle  =>  subtract from 1.5mS pulse
 * Therefore need only calculate for abs(angle)
 */
void  PCA9685_set_servo(uint32_t servo_no, int32_t angle)
{
uint8_t     PCA9685_i2c_packet[5];
int32_t    PWM_ON_time, PWM_OFF_time, pulse_change;
struct servo_data_s  *servo_data_pt;
uint8_t     PCA9685_chan_address;

    servo_data_pt = &servo_data[servo_no];
    if (servo_data_pt->state != DISABLED) {
        servo_data_pt->angle = angle;               // log requested angle
        
        pulse_change = ((abs(angle) * COUNT_1mS)/MAX_ANGLE);
        if (angle > 0) {
            PWM_OFF_time = MID_POINT_COUNT + pulse_change;
        } else {
            PWM_OFF_time = MID_POINT_COUNT - pulse_change;
        }
        PWM_OFF_time += servo_data_pt->pulse_offset;  // set OFF time
        PWM_ON_time = servo_data_pt->pulse_offset;    // set ON time
    } else {
        return;
    }

    if (servo_data_pt->sync == true) {
        return;    // delay execution of the move until later sync command
    }
    //
    // execute servo move
    //
    PCA9685_chan_address = PCA9685__LED0_ON_L + (servo_no << 2);
    PCA9685_i2c_packet[0] = PCA9685_chan_address;
    PCA9685_i2c_packet[1] = PWM_ON_time & 0xFF;
    PCA9685_i2c_packet[2] = (PWM_ON_time >> 8) & 0xFF;
    PCA9685_i2c_packet[3] = PWM_OFF_time & 0xFF;
    PCA9685_i2c_packet[4] = (PWM_OFF_time >> 8) & 0xFF;
    i2c_write_blocking(I2C_PORT, PCA9685_address, PCA9685_i2c_packet, 5, false);
}


void PCA9685_set_zero(uint32_t servo_no) 
{
uint8_t  PCA9685_i2c_packet[5];

    PCA9685_i2c_packet[0] = (uint8_t) PCA9685__LED0_ON_L + (servo_no << 2);
    PCA9685_i2c_packet[1] = 0x00;
    PCA9685_i2c_packet[2] = 0x00;
    PCA9685_i2c_packet[3] = 0x00;
    PCA9685_i2c_packet[4] = 0x00;
    i2c_write_blocking(I2C_PORT, PCA9685_address, PCA9685_i2c_packet, 5, false);
}

/**
 * @brief Set the servo channel object
 * 
 * @param servo_no 
 * @param servo_state 
 * @param servo_angle 
 * @param servo_sync 
 */
void    set_servo_channel(  uint8_t         servo_no,
                            servo_states_te servo_state,
                            int16_t         servo_angle,
                            bool            servo_sync
                            )
{
struct servo_data_s  *servo_data_pt;

    servo_data_pt = &servo_data[servo_no];

    servo_data_pt->state = servo_state;
    servo_data_pt->angle = servo_angle;
    servo_data_pt->sync  = servo_sync;
}
