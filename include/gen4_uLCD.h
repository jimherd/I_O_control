

/////////////////////////// gen4_uLCDC 07/08/2020 //////////////////////////////
//
//      Code based the 4D Systems gen4_uLCD open source code

//  This file is part of gen4_uLCDC:
//      gen4_uLCDC is free software: you can redistribute it and/or modify
//      it under the terms of the GNU Lesser General Public License as
//      published by the Free Software Foundation, either version 3 of the
//      License, or (at your option) any later version.
//  
//      gen4_uLCDC is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU Lesser General Public License for more details.
//  
//      You should have received a copy of the GNU Lesser General Public
//      License along with gen4_uLCDC.
//      If not, see <http://www.gnu.org/licenses/>.
//==============================================================================

#ifndef __GEN4_uLCD_H__
#define __GEN4_uLCD_H__

#include    "system.h"
#include    "stdint.h"
#include    "pico/stdlib.h"

// // Do not modify current values. Recommended settings.
// #define DISPLAY_TIMEOUT         2000
// #define AUTO_PING_CYCLE         1250

// Structure to store replys returned from a display
#define GEN4_uLCD_REPLY_SIZE        6

#define MAX_GEN4_uLCD_EVENTS    	16    // MUST be a power of 2

#define GEN4_uLCD_ACK               0x06
#define GEN4_uLCD_NAK               0x15
#define GEN4_uLCD_PING              0x80
#define GEN4_uLCD_READY             0x81
#define GEN4_uLCD_DISCONNECTED      0x82

#define MAX_COMMAND_DATA_BYTES	64

#define		NOS_GEN4_uLCD_CMDS  	8

#define		GEN4_uLCD_NOS_PINGS		1 // to set display

typedef enum {
	GEN4_uLCD_READ_OBJ,
	GEN4_uLCD_WRITE_OBJ,
	GEN4_uLCD_WRITE_STR,
	GEN4_uLCD_WRITE_STRU,
	GEN4_uLCD_WRITE_CONTRAST,
	GEN4_uLCD_REPORT_OBJ,
	GEN4_uLCD_REPORT_EVENT = 7,
} gen4_uLCD_Command_te;

typedef enum {
	GEN4_uLCD_OBJ_DIPSW,
	GEN4_uLCD_OBJ_KNOB,
	GEN4_uLCD_OBJ_ROCKERSW,
	GEN4_uLCD_OBJ_ROTARYSW,
	GEN4_uLCD_OBJ_SLIDER,
	GEN4_uLCD_OBJ_TRACKBAR,
	GEN4_uLCD_OBJ_WINBUTTON,
	GEN4_uLCD_OBJ_ANGULAR_METER,
	GEN4_uLCD_OBJ_COOL_GAUGE,
	GEN4_uLCD_OBJ_CUSTOM_DIGITS,
	GEN4_uLCD_OBJ_FORM,
	GEN4_uLCD_OBJ_GAUGE,
	GEN4_uLCD_OBJ_IMAGE,
	GEN4_uLCD_OBJ_KEYBOARD,
	GEN4_uLCD_OBJ_LED,
	GEN4_uLCD_OBJ_LED_DIGITS,
	GEN4_uLCD_OBJ_METER,
	GEN4_uLCD_OBJ_STRINGS,
	GEN4_uLCD_OBJ_THERMOMETER,
	GEN4_uLCD_OBJ_USER_LED,
	GEN4_uLCD_OBJ_VIDEO,
	GEN4_uLCD_OBJ_STATIC_TEXT,
	GEN4_uLCD_OBJ_SOUND,
	GEN4_uLCD_OBJ_TIMER,
	GEN4_uLCD_OBJ_SPECTRUM,
	GEN4_uLCD_OBJ_SCOPE,
	GEN4_uLCD_OBJ_TANK,
	GEN4_uLCD_OBJ_USERIMAGES,
	GEN4_uLCD_OBJ_PINOUTPUT,
	GEN4_uLCD_OBJ_PININPUT,
	GEN4_uLCD_OBJ_4DBUTTON,
	GEN4_uLCD_OBJ_ANIBUTTON,
	GEN4_uLCD_OBJ_COLORPICKER,
	GEN4_uLCD_OBJ_USERBUTTON,
	GEN4_uLCD_OBJ_MAGIC_RESERVED,
	GEN4_uLCD_OBJ_SMARTGAUGE,
	GEN4_uLCD_OBJ_SMARTSLIDER,
	GEN4_uLCD_OBJ_SMARTKNOB,
	GEN4_uLCD_OBJ_ILED_DIGITS_H,
	GEN4_uLCD_OBJ_IANGULAR_METER,
	GEN4_uLCD_OBJ_IGAUGE,
	GEN4_uLCD_OBJ_ILABELB,
	GEN4_uLCD_OBJ_IUSER_GAUGE,
	GEN4_uLCD_OBJ_IMEDIA_GAUGE,
	GEN4_uLCD_OBJ_IMEDIA_THERMOMETER,
	GEN4_uLCD_OBJ_ILED,
	GEN4_uLCD_OBJ_IMEDIA_LED,
	GEN4_uLCD_OBJ_ILED_DIGITS_L,
	GEN4_uLCD_OBJ_ILED_DIGITS,
	GEN4_uLCD_OBJ_INEEDLE,
	GEN4_uLCD_OBJ_IRULER,
	GEN4_uLCD_OBJ_ILED_DIGIT,
	GEN4_uLCD_OBJ_IBUTTOND,
	GEN4_uLCD_OBJ_IBUTTONE,
	GEN4_uLCD_OBJ_IMEDIA_BUTTON,
	GEN4_uLCD_OBJ_ITOGGLE_INPUT,
	GEN4_uLCD_OBJ_IDIAL,
	GEN4_uLCD_OBJ_IMEDIA_ROTARY,
	GEN4_uLCD_OBJ_IROTARY_INPUT,
	GEN4_uLCD_OBJ_ISWITCH,
	GEN4_uLCD_OBJ_ISWITCHB,
	GEN4_uLCD_OBJ_ISLIDERE,
	GEN4_uLCD_OBJ_IMEDIA_SLIDER,
	GEN4_uLCD_OBJ_ISLIDERH,
	GEN4_uLCD_OBJ_ISLIDERG,
	GEN4_uLCD_OBJ_ISLIDERF,
	GEN4_uLCD_OBJ_ISLIDERD,
	GEN4_uLCD_OBJ_ISLIDERC,
	GEN4_uLCD_OBJ_ILINEAR_INPUT
} gen4_uLCD_Object_te;

//==============================================================================
// Structure to hold a command to be sent to the display
//
// Although each command is of a fixed length; the seven commands have
// different lengths.  The last byte of the command is a checksum.
// The string to be sent to the display starts at "data[0]".
//
typedef struct gen4_uLCD_cmd_packet_ts {
	uint8_t		cmd_length;
    uint8_t		data[MAX_COMMAND_DATA_BYTES];
} gen4_uLCD_cmd_packet_ts;

//==============================================================================
// Structure to hold a reply from a READ_OBJ command

typedef struct gen4_uLCD_reply_packet_ts {
    uint8_t		cmd;
    uint8_t		object;
    uint8_t		index;
    uint8_t		data_msb;
    uint8_t     data_lsb;
	uint8_t		checksum;
} gen4_uLCD_reply_packet_ts;


//==============================================================================
// Function prototypes
//
void              uart1_sys_init(void);
error_codes_te    gen4_uLCD_init(void); 
void              reset_4D_display(void);
error_codes_te    gen4_uLCD_ReadObject(uint16_t object, uint16_t index);
error_codes_te    gen4_uLCD_WriteObject(uint16_t object, uint16_t index, uint16_t data);
error_codes_te    gen4_uLCD_WriteContrast(uint8_t value);
void     flush_RX_fifo(uart_inst_t *uart);
int32_t  get_active_form(void);

#endif  /* __GEN4_uLCD_H__ */



// typedef struct {
//     uint8_t         cmd;
//     uint8_t         index;
//     uint8_t         length;
// } MagicReportHeader;

// typedef union {
//     float floatValue;
//     int32_t longValue;
//     uint32_t ulongValue;
//     int16_t wordValue[2];
// } FloatLongFrame;

/////////////////////////////////////////////////////////////////////
// The gen4_uLCD frame definition
//
// The union allows the data to be referenced as an array of uint8_t
// or a structure of type FrameReportObj, eg
//
//    genieFrame f;
//    f.bytes[4];
//    f.reportObject.data_lsb
//
//    both methods get the same byte
//
// typedef union {
//     uint8_t             bytes[GEN4_uLCD_FRAME_SIZE];
//     FrameReportObj      reportObject;
// } genieFrame;

// typedef struct {
//     genieFrame    	frames[MAX_GEN4_uLCD_EVENTS];
//     uint8_t        	rd_index;
//     uint8_t        	wr_index;
//     uint8_t        	n_events;
// } EventQueueStruct;


// typedef void        (*UserEventHandlerPtr) (void);
// typedef void        (*UserMagicByteHandlerPtr) (uint8_t, uint8_t);
// typedef void        (*UserMagicDByteHandlerPtr) (uint8_t, uint8_t);
// //typedef void		(*UserDebuggerHandlerPtr) (char *);
// typedef void 		(*UserDebuggerHandlerPtr) (const char *, ...);

// EventQueueStruct EventQueue;

// UserEventHandlerPtr UserHandler;
// UserMagicByteHandlerPtr UserMagicByteHandler;
// UserMagicDByteHandlerPtr UserMagicDByteHandler;
// UserDebuggerHandlerPtr UserDebuggerHandler;

// // used internally by the library, do not modify!
// volatile bool     pendingACK; // prevent userhandler if waiting for ACK, and end recursion.
// volatile bool     pingRequest; // used internally by the library, do not touch.
// volatile uint8_t  recover_pulse; // pulse for offline autoping, use genie.recover(x) to change it from sketch.
// volatile bool     autoPing; // used internally by the library, do not touch.
// volatile uint16_t GEN4_uLCD_CMD_TIMEOUT; // force disconnection trigger if ACK times out
// volatile uint32_t autoPingTimer; // timer for autoPinger() function
// volatile bool     gen4_uLCD_detected; // display is online/offline state
// volatile uint32_t displayDetectTimer; // timer for lcd to be aware if connected
// volatile uint8_t  currentForm; // current form thats loaded
// volatile uint8_t  nakInj; // nak injection counter
// volatile uint8_t  badByteCounter; // used for disconnection/debugging purposes
// volatile uint32_t delayedCycles; // session protection if latency in user code
// volatile uint32_t display_uptime; // uptime of display
// volatile uint32_t ping_spacer; // prevent flooding the uart during recovery. non-blocking.
// volatile bool     genieStart;


// Public Functions
// bool 		genieBegin();

// uint16_t    genieWriteShortToIntLedDigits(uint16_t index, int16_t data);
// uint16_t    genieWriteFloatToIntLedDigits(uint16_t index, float data);
// uint16_t    genieWriteLongToIntLedDigits(uint16_t index, int32_t data);

// uint16_t    genieWriteStr(uint16_t index, char *string);
// uint16_t    genieWriteStrU(uint16_t index, uint16_t *string);
// uint16_t	genieWriteInhLabelDefault(uint16_t index);
// uint16_t    genieWriteInhLabel(uint16_t index, char *string);
// bool        genieEventIs(genieFrame * e, uint8_t cmd, uint8_t object, uint8_t index);
// uint16_t    genieGetEventData(genieFrame * e);
// bool        genieDequeueEvent(genieFrame * buff);

// uint8_t		genieDoEvents();
// void        genieAttachEventHandler(UserEventHandlerPtr handler);
// void        genieAttachMagicByteReader(UserMagicByteHandlerPtr handler);
// void        genieAttachMagicDoubleByteReader(UserMagicDByteHandlerPtr handler);
// void 		genieAttachDebugger(UserDebuggerHandlerPtr handler);
// bool 		genieOnline();
// uint32_t	genieUptime();
// uint8_t		genieCurrentForm();
// void		genieActivateForm(uint8_t form);
// void		genieRecover(uint8_t pulses);
// uint8_t		genieTimeout(uint16_t value);
// uint8_t		geniePing();
// uint16_t	genieEnableAutoPing(uint16_t interval);

// // gen4_uLCD Magic functions (ViSi-gen4_uLCD Pro Only)

// #define		genieWriteMagicBytes(index, bytes, len)		_genieWriteMagicBytes(index, bytes, len, 0);
// uint16_t    _genieWriteMagicBytes(uint16_t index, uint8_t *bytes, uint16_t len, uint8_t report);
// #define		genieWriteMagicDBytes(index, shorts, len)	_genieWriteMagicDBytes(index, shorts, len, 0);
// uint16_t    _genieWriteMagicDBytes(uint16_t index, uint16_t *shorts, uint16_t len, uint8_t report);

// uint8_t     genieGetNextByte(void);
// uint16_t    genieGetNextDoubleByte(void);

// // Private Functions

// uint8_t     genieEnqueueEvent(uint8_t * data);

// extern unsigned long millis(void);
// extern uint16_t genieGetByteCount(void);
// extern uint8_t genieGetByte(void);
// extern uint8_t geniePeekByte(void);
// extern void geniePutByte(uint8_t);

//#endif /* __GEN4_uLCD_H__ */
