
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

#define GEN4_uLCD_MAX_NOS_FORMS                 8
#define GEN4_uLCD_MAX_BUTTONS_PER_FORM          8
#define GEN4_uLCD_MAX_SWITCHES_PER_FORM     8
#define GEN4_uLCD_MAX_STRINGS_PER_FORM      8
#define GEN4_uLCD_MAX_STRING_CHARS              32  

#define     GEN4_uLCD_MAX_NOS_BUTTONS   64

typedef enum {
    OBJECT_UNUSED,
    OBJECT_DISABLED,
    OBJECT_ENABLED,
    OBJECT_SCAN_ENABLED,
}object_mode_te;

typedef enum {
    PRESSED,
    NOT_PRESSED,
} button_state_te;  // state held after button released

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
// Set of structures to hold details of the objects on the Gen4 LCD touch
// screen. "form_data_te" aggregates the data for all

typedef struct  {   // for WINBUTTON objects
    object_mode_te  object_mode;
    uint8_t         object_type;
    uint8_t         global_object_id;    // e.g. WINBUTTON0, WINBUTTON1, etc.
	int8_t	        button_value;
    int32_t         time_high;      // High time in time sample units
    button_state_te button_state;   // PRESSED, NOT_PRESSED
} touch_button_data_ts;

typedef struct  {   // for ISWITCHB objects
    object_mode_te  object_mode;          // enabled, disabled, unused
    uint8_t         object_type;
    uint8_t         global_object_id;    // e.g. ISWITCHB0, ISWITCHB1, etc.
	int8_t	        switch_value;
//    button_state_te button_state;   // PRESSED, NOT_PRESSED
} touch_switch_data_ts;

typedef struct {    // for STRINGS objects
    object_mode_te  object_mode;
    uint8_t         global_object_id;
    char            string[GEN4_uLCD_MAX_STRING_CHARS + 1];  // +1 for null terminator
} string_data_ts;

typedef struct {
    touch_button_data_ts    buttons[GEN4_uLCD_MAX_BUTTONS_PER_FORM];
    touch_switch_data_ts    switches[GEN4_uLCD_MAX_SWITCHES_PER_FORM];
    string_data_ts          strings[GEN4_uLCD_MAX_STRINGS_PER_FORM];
} form_data_ts;

//==============================================================================
// structure to hold number of actual objects per form.
// Only applies to WINBUTTON, ISWITCHB, and STRINGS objects

typedef struct  {
	uint32_t nos_buttons;
	uint32_t nos_switches;
	uint32_t nos_strings;
} nos_objects_per_form_te;

//==============================================================================
// Function prototypes
//==============================================================================

error_codes_te    gen4_uLCD_init(void);
void              uart1_sys_init(void);
void              flush_RX_fifo(uart_inst_t *uart);
void              reset_4D_display(void);

// basic API calls
error_codes_te    gen4_uLCD_ReadObject(uint16_t object, uint16_t global_index, uint32_t *result);
error_codes_te    gen4_uLCD_WriteObject(uint16_t object, uint16_t global_index, uint16_t data);
error_codes_te    gen4_uLCD_WriteString(uint16_t global_index, char *text);
error_codes_te    gen4_uLCD_WriteContrast(uint8_t value);

// higher level calls

error_codes_te    change_uLCD_form(int32_t new_form);
error_codes_te    read_uLCD_button(int32_t form, uint32_t object, uint32_t local_index, uint32_t *result);
error_codes_te    read_uLCD_switch(int32_t form, uint32_t object, uint32_t local_index, uint32_t *result);
error_codes_te    write_uLCD_string(int32_t form, uint32_t object, uint32_t local_index, struct string_buffer *buff_pt);
error_codes_te    get_uLCD_button_press(int32_t form, uint32_t object, uint32_t *result);

int32_t  get_uLCD_active_form(void);

void clear_button_state(uint32_t form, uint32_t local_index);
int32_t global_to_local_id(uint32_t form, uint32_t global_id);


#endif  /* __GEN4_uLCD_H__ */
