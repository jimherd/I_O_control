
#ifndef __GEN4_uLCD_H__
#define __GEN4_uLCD_H__

#include    "system.h"

// // Do not modify current values. Recommended settings.
// #define DISPLAY_TIMEOUT         2000
// #define AUTO_PING_CYCLE         1250

//==============================================================================
// Local display constants
//==============================================================================

#define MAX_COMMAND_DATA_BYTES	    80
#define	NOS_GEN4_uLCD_CMDS  	     8
#define GEN4_uLCD_REPLY_SIZE         6
#define DISPLAY_WAIT_US             25
#define UART_READ_TIME_OUT_uS  3000000  // 3 seconds

#define GEN4_uLCD_ACK               0x06
#define GEN4_uLCD_NAK               0x15
#define GEN4_uLCD_PING              0x80
#define GEN4_uLCD_READY             0x81
#define GEN4_uLCD_DISCONNECTED      0x82

//==============================================================================
// Definition of structures relevant to low level access to LCD display
//==============================================================================

//==============================================================================

// Structure to hold a command to be sent to the display
//
// Although each command is of a fixed length; the seven commands have
// different lengths.  The last byte of the command is a checksum.
// The string to be sent to the display starts at "data[0]".
//
typedef struct {
	uint8_t		cmd_length;
    uint8_t		data[MAX_COMMAND_DATA_BYTES];
} gen4_uLCD_cmd_packet_ts;

//==============================================================================
// Structure to hold a reply from a READ_OBJ command

typedef struct  {
    uint8_t	    cmd;
    uint8_t		object;
    uint8_t		index;
    uint8_t		data_msb;
    uint8_t     data_lsb;
	uint8_t		checksum;
} gen4_uLCD_reply_packet_ts;

//==============================================================================
// Structure to store replies returned from a display
// The reply is either an ACK or NACK character, or a 6 byte data packet

typedef union  {
	gen4_uLCD_reply_packet_ts		reply;
	uint8_t							packet[6];
} gen4_uLCD_reply_packet_tu;

enum {
    IGNORE_FORM = -1,
    POW_FORM    = GEN4_uLCD_FORM0,
    TEST1_FORM  = GEN4_uLCD_FORM1,
    TEST2_FORM  = GEN4_uLCD_FORM2,
};

//==============================================================================
// Function prototypes
//==============================================================================

error_codes_te  gen4_uLCD_init(void);
void              uart1_sys_init(void);
void              reset_4D_display(void);
error_codes_te    gen4_uLCD_ReadObject(uint16_t object, uint16_t global_index, uint32_t *result);
error_codes_te    gen4_uLCD_WriteObject(int32_t form, uint16_t object, uint16_t global_index, uint16_t data);
error_codes_te    gen4_uLCD_WriteContrast(uint8_t value);
error_codes_te    gen4_uLCD_WriteString(int32_t form, uint16_t global_index, uint8_t *text);
void     flush_RX_fifo(uart_inst_t *uart);
int32_t  get_active_form(void);
error_codes_te    change_form(int32_t new_form);
error_codes_te    read_uLCD_winbutton(int32_t form, uint32_t object, uint32_t local_index, uint32_t *result);
error_codes_te    read_uLCD_iswitchb(int32_t form, uint32_t object, uint32_t local_index, uint32_t *result);

#endif  /* __GEN4_uLCD_H__ */
