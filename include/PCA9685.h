/**
 * @file PCA9685.h
 * @author Jim Herd
 * @brief 
 * @version 0.1
 * @date 2023-03-14
 * *
 * @notes
 *      register definitions from  https://github.com/SMFSW/arm_i2c_drivers
 */

#ifndef __PCA9685_H__
#define __PCA9685_H__

#include    "system.h"
#include    "PCA9685.h"

#include    "pico/stdlib.h"

#define     PCA9685_address     0x40

#define		PCA9685_servo_frequency		50   // hertz
#define		PCA9685_50Hz_PRE_SCALER		138  // TUNED : calc = 123
  // refer to datasheet for calculation

#define		SERVO_TRIM_MIN		110
#define     SERVO_TRIM_MAX		590

#define		MID_POINT_COUNT		307
#define		COUNT_1mS			205
#define		MAX_ANGLE			 90

typedef enum  {SERVO, MOTOR} servo_type_te;
enum {R_EYEBALL, L_EYEBALL, R_EYE_LID, L_EYE_LID, R_EYE_BROW, L_EYE_BROW, MOUTH};

#define		NOS_SERVOS	(MOUTH + 1)

//==============================================================================
// structure to hold servo specific data

typedef enum {ABS_MOVE, ABS_MOVE_SYNC, SPEED_MOVE, SPEED_MOVE_SYNC, RUN_SYNC_MOVES, STOP, STOP_ALL} servo_commands_te;

typedef enum {DISABLED, DORMANT, DELAY, MOVE, TIMED_MOVE, MOVE_SYNC_HOLD, TIMED_MOVE_SYNC_HOLD} servo_states_te;

struct servo_data_s {
	servo_states_te	state;
	bool			sync;
	servo_type_te	type;
	int16_t			angle;			// current value
	int16_t			angle_target;
	int16_t			speed_value;
	int16_t			init_angle;		// power-on state
	bool			flip;
	int16_t			angle_min, angle_max;
	uint16_t		pulse_offset;
	uint32_t		counter;
	float			gradient;
	float   		y_intercept;
	uint32_t		t_end;
};

//==============================================================================
// register set for PCA9685 device

typedef enum  PCA9685_reg_map {
	PCA9685__MODE1 = 0,				//!< register MODE1
	PCA9685__MODE2,					//!< register MODE2
	PCA9685__SUBADR1,				//!< register SUBADR1
	PCA9685__SUBADR2,				//!< register SUBADR2
	PCA9685__SUBADR3,				//!< register SUBADR3
	PCA9685__ALLCALLADR,			//!< register ALLCALLADR
	PCA9685__LED0_ON_L,				//!< register LED 0 ON Low
	PCA9685__LED0_ON_H,				//!< register LED 0 ON High
	PCA9685__LED0_OFF_L,			//!< register LED 0 OFF Low
	PCA9685__LED0_OFF_H,			//!< register LED 0 OFF High
	PCA9685__LED1_ON_L,				//!< register LED 1 ON Low
	PCA9685__LED1_ON_H,				//!< register LED 1 ON High
	PCA9685__LED1_OFF_L,			//!< register LED 1 OFF Low
	PCA9685__LED1_OFF_H,			//!< register LED 1 OFF High
	PCA9685__LED2_ON_L,				//!< register LED 2 ON Low
	PCA9685__LED2_ON_H,				//!< register LED 2 ON High
	PCA9685__LED2_OFF_L,			//!< register LED 2 OFF Low
	PCA9685__LED2_OFF_H,			//!< register LED 2 OFF High
	PCA9685__LED3_ON_L,				//!< register LED 3 ON Low
	PCA9685__LED3_ON_H,				//!< register LED 3 ON High
	PCA9685__LED3_OFF_L,			//!< register LED 3 OFF Low
	PCA9685__LED3_OFF_H,			//!< register LED 3 OFF High
	PCA9685__LED4_ON_L,				//!< register LED 4 ON Low
	PCA9685__LED4_ON_H,				//!< register LED 4 ON High
	PCA9685__LED4_OFF_L,			//!< register LED 4 OFF Low
	PCA9685__LED4_OFF_H,			//!< register LED 4 OFF High
	PCA9685__LED5_ON_L,				//!< register LED 5 ON Low
	PCA9685__LED5_ON_H,				//!< register LED 5 ON High
	PCA9685__LED5_OFF_L,			//!< register LED 5 OFF Low
	PCA9685__LED5_OFF_H,			//!< register LED 5 OFF High
	PCA9685__LED6_ON_L,				//!< register LED 6 ON Low
	PCA9685__LED6_ON_H,				//!< register LED 6 ON High
	PCA9685__LED6_OFF_L,			//!< register LED 6 OFF Low
	PCA9685__LED6_OFF_H,			//!< register LED 6 OFF High
	PCA9685__LED7_ON_L,				//!< register LED 7 ON Low
	PCA9685__LED7_ON_H,				//!< register LED 7 ON High
	PCA9685__LED7_OFF_L,			//!< register LED 7 OFF Low
	PCA9685__LED7_OFF_H,			//!< register LED 7 OFF High
	PCA9685__LED8_ON_L,				//!< register LED 8 ON Low
	PCA9685__LED8_ON_H,				//!< register LED 8 ON High
	PCA9685__LED8_OFF_L,			//!< register LED 8 OFF Low
	PCA9685__LED8_OFF_H,			//!< register LED 8 OFF High
	PCA9685__LED9_ON_L,				//!< register LED 9 ON Low
	PCA9685__LED9_ON_H,				//!< register LED 9 ON High
	PCA9685__LED9_OFF_L,			//!< register LED 9 OFF Low
	PCA9685__LED9_OFF_H,			//!< register LED 9 OFF High
	PCA9685__LED10_ON_L,			//!< register LED 10 ON Low
	PCA9685__LED10_ON_H,			//!< register LED 10 ON High
	PCA9685__LED10_OFF_L,			//!< register LED 10 OFF Low
	PCA9685__LED10_OFF_H,			//!< register LED 10 OFF High
	PCA9685__LED11_ON_L,			//!< register LED 11 ON Low
	PCA9685__LED11_ON_H,			//!< register LED 11 ON High
	PCA9685__LED11_OFF_L,			//!< register LED 11 OFF Low
	PCA9685__LED11_OFF_H,			//!< register LED 11 OFF High
	PCA9685__LED12_ON_L,			//!< register LED 12 ON Low
	PCA9685__LED12_ON_H,			//!< register LED 12 ON High
	PCA9685__LED12_OFF_L,			//!< register LED 12 OFF Low
	PCA9685__LED12_OFF_H,			//!< register LED 12 OFF High
	PCA9685__LED13_ON_L,			//!< register LED 13 ON Low
	PCA9685__LED13_ON_H,			//!< register LED 13 ON High
	PCA9685__LED13_OFF_L,			//!< register LED 13 OFF Low
	PCA9685__LED13_OFF_H,			//!< register LED 13 OFF High
	PCA9685__LED14_ON_L,			//!< register LED 14 ON Low
	PCA9685__LED14_ON_H,			//!< register LED 14 ON High
	PCA9685__LED14_OFF_L,			//!< register LED 14 OFF Low
	PCA9685__LED14_OFF_H,			//!< register LED 14 OFF High
	PCA9685__LED15_ON_L,			//!< register LED 15 ON Low
	PCA9685__LED15_ON_H,			//!< register LED 15 ON High
	PCA9685__LED15_OFF_L,			//!< register LED 15 OFF Low
	PCA9685__LED15_OFF_H,			//!< register LED 15 OFF High
	PCA9685__ALL_LED_ON_L = 0xFA,	//!< register ALL LED ON Low
	PCA9685__ALL_LED_ON_H,			//!< register ALL LED ON High
	PCA9685__ALL_LED_OFF_L,			//!< register ALL LED OFF Low
	PCA9685__ALL_LED_OFF_H,			//!< register ALL LED OFF High
	PCA9685__PRE_SCALE,				//!< register PRE_SCALE
	PCA9685__TestMode				//!< register TestMode
} PCA9685_reg;

//==============================================================================
/*!\union uPCA9685_REG__MODE1
** \brief Union for MODE1 register of PCA9685
**/
typedef union  uPCA9685_REG__MODE1 {
	uint8_t Byte;
	struct __attribute__((__packed__)) {
		uint8_t ALLCALL	:1;		//!< All CALL address acknowledgment
		uint8_t SUB3	:1;		//!< Sub-address 3 acknowledgment
		uint8_t SUB2	:1;		//!< Sub-address 2 acknowledgment
		uint8_t SUB1	:1;		//!< Sub-address 1 acknowledgment
		uint8_t SLEEP	:1;		//!< 0: Normal, 1: Low Power
		uint8_t AI		:1;		//!< Auto-increment bit (0: Disabled, 1: Enabled)
		uint8_t EXTCLK	:1;		//!< External Clock (0: Internal clock, 1: External clock)
		//!\warning This bit is a 'sticky bit', that is, it cannot be cleared by writing a logic 0 to it. The EXTCLK bit can only be cleared by a power cycle or software reset.
		uint8_t RESTART	:1;		//!< 0: Restart disabled, 1: Restart enabled
		//!\warning User writes logic 1 to this bit to clear it to logic 0. A user write of logic 0 will have no effect.
	} Bits;
} uPCA9685_REG__MODE1;

/*!\union uPCA9685_REG__MODE2
** \brief Union for MODE2 register of PCA9685
**/
typedef union uPCA9685_REG__MODE2 {
	uint8_t Byte;
	struct __attribute__((__packed__)) {
		uint8_t 		OUTNE	:2;
		uint8_t			OUTDRV	:1;		//!< 0: Open drain, 1: Totem Pole
		uint8_t     	OCH		:1;		//!< 0: update on STOP, 1: update on ACK
		uint8_t			INVRT	:1;		//!< 0: Output logic not inverted, 1: Output logic inverted
		uint8_t 				:3;
	} Bits;
} uPCA9685_REG__MODE2;


/*!\union uPCA9685_REG__DUTY
** \brief Union for Duty Cycle registers of PCA9685
**/
typedef union  uPCA9685_REG__DUTY {
	uint16_t Word;
	struct __attribute__((__packed__)) {
		uint8_t LSB;				//!< Less significant byte
		uint8_t MSB;				//!< Most significant byte
	} Bytes;
	struct __attribute__((__packed__)) {
		uint16_t 	VAL		:12;	//!< Value
		uint16_t	FULL	:1;		//!< Full bit
		uint16_t 			:3;
	} Bits;
} uPCA9685_REG__DUTY;

//  Bit masks for mode1 register

#define		MODE_SLEEP_MASK			0x10
#define		PCA9685_MODE1_RESTART	0x80
#define		PCA9685_MODE1_AUTO_INC  0x20

//==============================================================================
// function templates

void init_PCA9685_servo_IO(void);
void write_PCA9685_register(uint8_t reg_number, uint8_t data_byte);
uint8_t read_PCA9685_register(uint8_t reg_number);
void  PCA9685_set_sleep(bool mode);
void  PCA9685_set_auto_increment(bool mode);
void  PCA9685_reset(void);
void PCA9685_set_servo_freq(void);
void  PCA9685_set_servo(uint32_t servo_no, int32_t position);
void  PCA9685_set_zero(uint32_t servo_no);
void  set_servo_move(  uint8_t            servo_no,
                       servo_commands_te  servo_state,
                       int16_t            servo_angle,
                       bool               servo_sync
                    );
void    set_servo_speed_move( uint8_t             servo_no,
    						  servo_commands_te   servo_state,
                              int16_t             servo_angle,
                              int16_t             time_for_move,  // units of 100ms
                              bool                servo_sync 
							);

#endif