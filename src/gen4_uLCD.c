/**
 * @file gen4_uLCD.c
 * @author Jim Herd 
 * @brief 
 * @date 2023-11-08
 * 
 * @notes
 *    This file contains the code to control the 4D Systems Gen4 uLCD display. 
 *    The display is controlled by sending commands to the display via the UART1 port.
 *    The display is polled for button presses and other events.
 *    The display is used to show forms, buttons, switches and strings.
 * 
 *    Access to a screen object requires up to three parameters
 *       a. FORM number  (>=0)
 * 	 	 b. Object type (e.g. WINBUTTON, ISWITCHB, etc.)
 *       c. Global Object index (e.g. WINBUTTON0, ISWITCHB1, etc.)
 *          - index based on the collection of forms
 *          - vluse held in "form" data structure
 *	     d. Local Object index (0, 1, 2, etc.)
 *          - index based on an individual form
 *          - index is assumed by the posiion of the object in the "form" data structure
 *          - used in high level code; converted to Global ID in low level code
 */

#include	"system.h"

#include	<stdio.h>
#include	<stdint.h>
#include    <string.h>
#include	"pico/stdlib.h"

#include    "hardware/uart.h"

#include	"FreeRTOS.h"
#include    "timers.h"


#include	"externs.h"
#include	"gen4_uLCD.h"

//==============================================================================
// Global variables
//==============================================================================

gen4_uLCD_cmd_packet_ts    uLCD_cmd;
gen4_uLCD_reply_packet_tu  uLCD_reply_data;
bool 		gen4_uLCD_detected, gen4_uLCD_test_apply;
int32_t		gen4_uLCD_current_form;

form_data_ts    form_data[GEN4_uLCD_MAX_NOS_FORMS] = {
    {   // form 0
        .buttons = {
            {OBJECT_SCAN_ENABLED, GEN4_uLCD_OBJ_WINBUTTON, GEN4_uLCD_WINBUTTON0, 0, 0, NOT_PRESSED},
            {OBJECT_SCAN_ENABLED, GEN4_uLCD_OBJ_WINBUTTON, GEN4_uLCD_WINBUTTON1, 0, 0, NOT_PRESSED},
        },
        .switches = {
        },
        .strings = {
            {OBJECT_ENABLED, GEN4_uLCD_STRING0, "**********"},
            {OBJECT_ENABLED, GEN4_uLCD_STRING1, "Pi the robot"},
        },
    },
    {   // form 1
        .buttons = {
            {OBJECT_SCAN_ENABLED, GEN4_uLCD_OBJ_WINBUTTON, GEN4_uLCD_WINBUTTON2, 0, 0, NOT_PRESSED},
            {OBJECT_SCAN_ENABLED, GEN4_uLCD_OBJ_WINBUTTON, GEN4_uLCD_WINBUTTON3, 0, 0, NOT_PRESSED},
            {OBJECT_SCAN_ENABLED, GEN4_uLCD_OBJ_WINBUTTON, GEN4_uLCD_WINBUTTON4, 0, 0, NOT_PRESSED},
        },
        .switches = {
            {OBJECT_ENABLED, GEN4_uLCD_OBJ_ISWITCHB, 0},
            {OBJECT_ENABLED, GEN4_uLCD_OBJ_ISWITCHB, 0},
            {OBJECT_ENABLED, GEN4_uLCD_OBJ_ISWITCHB, 0},
            {OBJECT_ENABLED, GEN4_uLCD_OBJ_ISWITCHB, 0},
        },
        .strings = {},
    },
    {   // form 2
        .buttons = {
            {OBJECT_SCAN_ENABLED, GEN4_uLCD_OBJ_WINBUTTON, GEN4_uLCD_WINBUTTON5, 0, 0, NOT_PRESSED},
            {OBJECT_SCAN_ENABLED, GEN4_uLCD_OBJ_WINBUTTON, GEN4_uLCD_WINBUTTON6, 0, 0, NOT_PRESSED},
            {OBJECT_SCAN_ENABLED, GEN4_uLCD_OBJ_WINBUTTON, GEN4_uLCD_WINBUTTON7, 0, 0, NOT_PRESSED},
        },
        .switches = {
            {OBJECT_ENABLED, GEN4_uLCD_OBJ_ISWITCHB, 0},
            {OBJECT_ENABLED, GEN4_uLCD_OBJ_ISWITCHB, 0},
            {OBJECT_ENABLED, GEN4_uLCD_OBJ_ISWITCHB, 0},
            {OBJECT_ENABLED, GEN4_uLCD_OBJ_ISWITCHB, 0},
        },
        .strings = {},
    },
};

nos_objects_per_form_te		nos_object[NOS_FORMS] = { 0 };  // set to zero

//==============================================================================
// Display functions
//==============================================================================

void uart1_sys_init(void)
{
    // ring_buffer_in.in_pt   = 0;
    // ring_buffer_in.out_pt  = 0;
    // ring_buffer_in.count   = 0;
    // ring_buffer_out.in_pt  = 0;
    // ring_buffer_out.out_pt = 0;
    // ring_buffer_out.count  = 0;

	uart_init(uart1, UART1_BAUD_RATE);
    gpio_set_function(UART1_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART1_RX_PIN, GPIO_FUNC_UART);
    uart_set_hw_flow(uart1, false, false);
    uart_set_format(uart1, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(uart1, true);

    // irq_set_exclusive_handler(UART_IRQ, uart_interrupt_handler);
    // irq_set_enabled(UART0_IRQ, true);

    // hw_set_bits(&UART->imsc, UART_UARTIMSC_RXIM_BITS | UART_UARTIMSC_RTIM_BITS);

    // gpio_init(DISPLAY_RESET_PIN);
    // gpio_set_dir(DISPLAY_RESET_PIN, GPIO_IN);
    // gpio_put(DISPLAY_RESET_PIN, 0);
    // gpio_disable_pulls(DISPLAY_RESET_PIN);

	reset_4D_display();
	flush_RX_fifo(uart1);
	vTaskDelay(3000);
}
//==============================================================================
/**
 * @brief generate a reset pulse for the display
 * 
 * @return * void 
 * 
 * @note
 *      Display reset line should be driven by an open-drain output,
 *      but this is not available on the rp2040.  However, it can be
 *      approximated by switching the reset line from input to output
 *      and finally back to input.
 */
void reset_4D_display(void)
{

	START_PULSE; STOP_PULSE;
    gpio_put(DISPLAY_RESET_PIN, 0);
    gpio_set_dir(DISPLAY_RESET_PIN, GPIO_OUT);
    busy_wait_us(DISPLAY_WAIT_US);
    gpio_set_dir(DISPLAY_RESET_PIN, GPIO_IN);
}

//==============================================================================

error_codes_te   gen4_uLCD_init(void) 
{
error_codes_te status;
uint32_t       count, i, j;

	reset_4D_display();

	gen4_uLCD_current_form = -1;
	gen4_uLCD_detected = false;
//
// Precalculate number of each of the folloeing types per form
//     WINBUTTON, ISWITCHB, and STRINGS
//

	for (i = 0; i < NOS_FORMS; i++ ) {
		count = 0;
		for (j = 0; j < GEN4_uLCD_MAX_BUTTONS_PER_FORM; j++) {
			if (form_data[i].buttons[j].object_mode != OBJECT_UNUSED) {
				count++;
			}
		}
		nos_object[i].nos_buttons = count;
		count = 0;
		for (j = 0; j < GEN4_uLCD_MAX_BUTTONS_PER_FORM; j++) {
			if (form_data[i].switches[j].object_mode != OBJECT_UNUSED) {
				count++;
			}
		}
		nos_object[i].nos_strings = count;
		count = 0;
		for (j = 0; j < GEN4_uLCD_MAX_BUTTONS_PER_FORM; j++) {
			if (form_data[i].strings[j].object_mode != OBJECT_UNUSED) {
				count++;
			}
		}
		nos_object[i].nos_strings = count;
	}
//
// Use some WriteContrast command as ping. 
// Disable display check this one time.
//
	gen4_uLCD_test_apply = false;
	for(int i=0 ; i < GEN4_uLCD_NOS_PINGS ; i++) {
		status = gen4_uLCD_WriteContrast(5);
		if (status != OK){
			gen4_uLCD_detected = false;
			return status;   // exit if error
		}
		vTaskDelay(1000);
	}
	gen4_uLCD_detected = true;
	gen4_uLCD_test_apply = true;
//
// Check with a final WriteContrast
//
	status = gen4_uLCD_WriteContrast(5);
	if (status != OK) {
		gen4_uLCD_detected = false;
	}
	return status;
}

//==============================================================================
/**
 * @brief Clear RX fifo 
 * 
 * @param uart 
 */
void flush_RX_fifo(uart_inst_t *uart)
{
uint8_t RX_byte;

	for(;;) {
		if (uart_is_readable(uart) == true) {
			RX_byte = uart_getc(uart);
		} else {
			break;
		}
	}
}
//==============================================================================
// Low level API function calls for uLCD display
//==============================================================================
// READ_OBJ			(0x00) Read data from display object
// WRITE_OBJ		(0x01) Write data to display object
// WRITE_STR		(0x02) Write ASCII string to display string object
// WRITE_USTR		(0x03) Write UNICODE string to display string object [unused]
// WRITE_CONTRAST	(0x04) Change screen contrast
// REPORT_OBJ		(0x05) Reply from a READ_OBJ command (Display -> Host)
// REPORT_EVENT   	(0x07) Report from async event (Display->Host)		 [UNUSED]
//==============================================================================
/**
 * @brief Execute a read object command (command 0x00)
 * 
 * @param object 	object type. List is in enum gen4_uLCD_Object_te
 * @param index     Generated by VisiGenie graphics design package
 * @return uint8_t  Error from 
 */
error_codes_te gen4_uLCD_ReadObject(uint16_t object, uint16_t global_index, uint32_t *result) 
{
uint8_t  checksum, reply_byte, i;
error_codes_te status;

	if (gen4_uLCD_test_apply == true) {
		if (gen4_uLCD_detected == false) {
			return GEN4_uLCD_NOT_DETECTED;
		}
	}
	xSemaphoreTake(gen4_uLCD_MUTEX_access, portMAX_DELAY);
	 uLCD_cmd.cmd_length = display_cmd_info[GEN4_uLCD_READ_OBJ].length;

	 uLCD_cmd.data[0] = GEN4_uLCD_READ_OBJ;
	 uLCD_cmd.data[1] = object;
	 uLCD_cmd.data[2] = global_index;

	checksum = 0;
	for (i=0 ; i < (uLCD_cmd.cmd_length - 1) ; i++) {
		checksum ^= uLCD_cmd.data[i];
	}
	uLCD_cmd.data[3] = checksum;
	uart_write_blocking(uart1, &uLCD_cmd.data[0], uLCD_cmd.cmd_length);
//
// Reply from the display is either an NACK or a 6 byte data packet

	if (uart_is_readable_within_us(uart1, UART_READ_TIME_OUT_uS) == true) {
		reply_byte = uart_getc(uart1);		// read first byte to check for NACK
		if (reply_byte == GEN4_uLCD_NAK) {
			status =  GEN4_uLCD_READ_OBJ_FAIL;
		} else {
			uLCD_reply_data.reply.cmd = reply_byte;
			uart_read_blocking(uart1, &uLCD_reply_data.reply.object, (sizeof(gen4_uLCD_reply_packet_ts) - 1));
			checksum = 0;
			for (i=0 ; i < (sizeof(gen4_uLCD_reply_packet_ts) - 1) ; i++) {
				checksum = checksum ^ uLCD_reply_data.packet[i];
			}
			*result = (((uint32_t)(uLCD_reply_data.packet[3]) << 8 ) & 0xFF00) + (uint32_t)(uLCD_reply_data.packet[4]);
			status = OK;
		} 
	} else {
		status =  GEN4_uLCD_READ_OBJ_TIMEOUT;
	}
	xSemaphoreGive(gen4_uLCD_MUTEX_access);
	return status;
}

//==============================================================================
/**
 * @brief Execute write object command (command 0x01)
 * 
 * @param object 
 * @param index 
 * @param data 
 * @return error_codes_te 
 */
error_codes_te   gen4_uLCD_WriteObject(uint16_t object, uint16_t global_index, uint16_t data) 
{
uint8_t  checksum, reply_byte;
error_codes_te   status;

     if (gen4_uLCD_test_apply == true) {
		if (gen4_uLCD_detected == false) {
			return GEN4_uLCD_NOT_DETECTED;
		}
	}
	xSemaphoreTake(gen4_uLCD_MUTEX_access, portMAX_DELAY);
	 uLCD_cmd.cmd_length = display_cmd_info[GEN4_uLCD_WRITE_OBJ].length;

	 uLCD_cmd.data[0] = GEN4_uLCD_WRITE_OBJ;
	 uLCD_cmd.data[1] = object;
	 uLCD_cmd.data[2] = global_index;
	 uLCD_cmd.data[3] = highByte16(data);
	 uLCD_cmd.data[4] = lowByte16(data);

	checksum = 0;
	for (int i=0 ; i < (uLCD_cmd.cmd_length - 1) ; i++) {
		checksum ^= uLCD_cmd.data[i];
	}
	uLCD_cmd.data[5] = checksum;
//
// Send packet to uart. The command packets are small, therefore, the FIFO in the uart
// should cause the transfer to be much faster than the baud rate timing would
// suggest.
//
	uart_write_blocking(uart1, &uLCD_cmd.data[0], uLCD_cmd.cmd_length);
//
// Reply from the display is either an ACK or NACK character
//
	if (uart_is_readable_within_us(uart1, UART_READ_TIME_OUT_uS) == true) {
		reply_byte = uart_getc(uart1);
		if (reply_byte == GEN4_uLCD_ACK) {
			status = OK;
		} else {
			status =  GEN4_uLCD_WRITE_OBJ_FAIL;
		}
	} else {
		status =  GEN4_uLCD_WRITE_OBJ_TIMEOUT;
	}
	xSemaphoreGive(gen4_uLCD_MUTEX_access);
	return status;
}

//==============================================================================
/**
 * @brief Execute write string command (0x02)
 * 
 * @param object 
 * @param index 			String object index
 * @param text 		   		pointer to ASCII null terminated string
 * @return error_codes_te 
 */
uint8_t  string_too_long[] = "string too long";

error_codes_te    gen4_uLCD_WriteString(uint16_t global_index, char *text) 
{
uint8_t   checksum, reply_byte, text_length, *str_pt;
error_codes_te   status;

	if (gen4_uLCD_test_apply == true) {
		if (gen4_uLCD_detected == false) {
			return GEN4_uLCD_NOT_DETECTED;
		}
	}
	str_pt = text;
	text_length = strlen(text);   // does not include terminating '\0' character
	if (text_length > MAX_GEN4_uLCD_WRITE_STR_SIZE) {
		// replace string with error message
		str_pt = string_too_long;
		text_length = strlen(str_pt);
		// return GEN4_uLCD_WRITE_STR_TOO_BIG;
	}

	xSemaphoreTake(gen4_uLCD_MUTEX_access, portMAX_DELAY);

	uLCD_cmd.data[0] = GEN4_uLCD_WRITE_STR;
	uLCD_cmd.data[1] = global_index;
	uLCD_cmd.cmd_length = text_length + 1 + display_cmd_info[GEN4_uLCD_WRITE_STR].length;
	uLCD_cmd.data[2] = text_length + 1;
	strncpy(&uLCD_cmd.data[3], str_pt, MAX_GEN4_uLCD_WRITE_STR_SIZE);
	checksum = 0;
	for (int i=0 ; i < (uLCD_cmd.cmd_length - 1) ; i++) {
		checksum ^= uLCD_cmd.data[i];
	}
    uLCD_cmd.data[uLCD_cmd.cmd_length - 1] = checksum;

    uart_write_blocking(uart1, &uLCD_cmd.data[0], uLCD_cmd.cmd_length);
//
// Reply from the display is either an ACK or NACK character
// uses simple time-out
//
	if (uart_is_readable_within_us(uart1, UART_READ_TIME_OUT_uS) == true) {
		reply_byte = uart_getc(uart1);
		if (reply_byte == GEN4_uLCD_ACK) {
			status = OK;
		} else {
			status = GEN4_uLCD_WRITE_STRING_FAIL;
		}
	 } else {
	 	status = GEN4_uLCD_WRITE_STRING_TIMEOUT;
	 }
	 xSemaphoreGive(gen4_uLCD_MUTEX_access);
	 // retain copy of string in form data structure
	 strncpy(form_data[get_uLCD_active_form()].strings[global_index].string, str_pt, MAX_GEN4_uLCD_WRITE_STR_SIZE);

	 return status;
}

//==============================================================================
/**
 * @brief  Execute write contrast command (command 0x04)
 * 
 * @param value 
 * @return error_codes_te 
 */
error_codes_te  gen4_uLCD_WriteContrast(uint8_t value)
{
uint8_t   checksum, reply_byte;
error_codes_te   status;

	if (gen4_uLCD_test_apply == true) {
		if (gen4_uLCD_detected == false) {
			return GEN4_uLCD_NOT_DETECTED;
		}
	}
	xSemaphoreTake(gen4_uLCD_MUTEX_access, portMAX_DELAY);
	uLCD_cmd.cmd_length = display_cmd_info[GEN4_uLCD_WRITE_CONTRAST].length;
	uLCD_cmd.data[0] = GEN4_uLCD_WRITE_CONTRAST;
	uLCD_cmd.data[1] = value;
	checksum = 0;
	for (int i=0 ; i < (uLCD_cmd.cmd_length - 1) ; i++) {
		checksum ^= uLCD_cmd.data[i];
	}
	uLCD_cmd.data[2] = checksum;
	uart_write_blocking(uart1, &uLCD_cmd.data[0], uLCD_cmd.cmd_length);
//
// Reply from the display is either an ACK or NACK character
// uses simple time-out
//
	if (uart_is_readable_within_us(uart1, UART_READ_TIME_OUT_uS) == true) {
		reply_byte = uart_getc(uart1);
		if (reply_byte == GEN4_uLCD_ACK) {
			status = OK;
		} else {
			status = GEN4_uLCD_WRITE_CONTRAST_FAIL;
		}
	 } else {
	 	status = GEN4_uLCD_WRITE_CONTRAST_TIMEOUT;
	 }
	 xSemaphoreGive(gen4_uLCD_MUTEX_access);
	 return status;
}

//==============================================================================
// High level function calls for uLCD display
//==============================================================================

//==============================================================================
/**
 * @brief Change the active form
 * 
 * @param 	new_form 	0 to NOS_FORMS - 1
 * @return 	error_codes_te 
 * 
 * @note 	Update the string objects on this form
 *          (Docs suggest that sting display to not retained when form is )
 */
error_codes_te  change_uLCD_form(int32_t new_form) 
{
error_codes_te status;

	if ((new_form > -1) && (new_form < NOS_FORMS)) {
		status = gen4_uLCD_WriteObject(GEN4_uLCD_OBJ_FORM, new_form, 0);
        if (status == OK) {
            gen4_uLCD_current_form = new_form;
        }
    } else {
		status = GEN4_uLCD_CMD_BAD_FORM_INDEX;
	}
	// update string objects on form
	if (nos_object[new_form].nos_strings > 0){
		for (int i = 0; i < nos_object[new_form].nos_strings; i++){
			gen4_uLCD_WriteString(form_data[gen4_uLCD_current_form].strings[i].global_object_id, 
			                      form_data[gen4_uLCD_current_form].strings[i].string);
		}
	}
}

//==============================================================================
/**
 * @brief Get the active form object
 * 
 * @return uint32_t -1 = no form active
 */
inline  int32_t get_uLCD_active_form(void) 
{
	return gen4_uLCD_current_form;
}

//==============================================================================
error_codes_te    read_uLCD_switch(int32_t form, uint32_t object, uint32_t local_index, uint32_t *result)
{
uint32_t switch_result, active_form, index;
error_codes_te status;

	active_form = get_uLCD_active_form();
	if ( form != active_form) {
		return GEN4_uLCD_BUTTON_FORM_INACTIVE;
	} else {
		status = OK;
	}
	// check if object is on the active form
	if (form_data[form].switches[local_index].object_mode == OBJECT_UNUSED) {
		return GEN4_uLCD_SWITCH_OBJECT_NOT_USED;
	}
	
// read value
	index = form_data[form].switches[local_index].global_object_id;
    status = gen4_uLCD_ReadObject(object, index, &switch_result);
	*result = switch_result;
	return status;
}
//==============================================================================

error_codes_te    read_uLCD_button(int32_t form, uint32_t object, uint32_t local_index, uint32_t *result)
{
uint32_t button_result, active_form, index;
error_codes_te status;

	active_form = get_uLCD_active_form();
	if ( form != active_form) {
		return GEN4_uLCD_BUTTON_FORM_INACTIVE;
	} else {
		status = OK;
	}
// check if object is on the active form
	if (form_data[form].buttons[local_index].object_mode == OBJECT_UNUSED) {
		return GEN4_uLCD_BUTTON_OBJECT_NOT_USED;
	}
	
// read value
	index = form_data[form].buttons[local_index].global_object_id;
    status = gen4_uLCD_ReadObject(object, index, &button_result);
	*result = button_result;
	return status;
}

//==============================================================================
error_codes_te    write_uLCD_string(int32_t form, uint32_t object, uint32_t local_index, struct string_buffer *buff_pt) 
{
error_codes_te status;
uint32_t	active_form, global_index;

	// Check that form is the active form
	active_form = get_uLCD_active_form();
	if ( form != active_form) {
		return GEN4_uLCD_BUTTON_FORM_INACTIVE;
	} else {
		status = OK;
	}
	// check if object is on the active form
	if (form_data[form].strings[local_index].object_mode == OBJECT_UNUSED) {
		return GEN4_uLCD_BUTTON_OBJECT_NOT_USED;
	}	
	// write string
	global_index = form_data[form].strings[local_index].global_object_id;
	status = gen4_uLCD_WriteString(global_index, buff_pt->buffer);
	if (status != OK) {
		return status;
	}
	// log string on form_data structure
	strncpy(form_data[form].strings[local_index].string, buff_pt->buffer, GEN4_uLCD_MAX_STRING_CHARS);
	
	return OK;
}

//==============================================================================
error_codes_te    get_uLCD_button_press(int32_t form, uint32_t object, uint32_t *result)
{
error_codes_te status;
uint32_t	   active_form, global_index;

	// Check that form is the active form
	active_form = get_uLCD_active_form();
	if ( form != active_form) {
		return GEN4_uLCD_BUTTON_FORM_INACTIVE;
	} else {
		status = OK;
	}
	// update button objects on form starting at index 0
	*result = -1;   // indicates that no buttons on the form have been pressed
	if (nos_object[form].nos_buttons > 0){
		for (int i = 0; i < nos_object[form].nos_buttons; i++){
			if (form_data[form].buttons[i].button_state == PRESSED) {
				*result = i;
			} else {
				continue;    // check next button
			}
		}
	}
	return OK;
}

void inline clear_button_state(uint32_t form, uint32_t local_index) {
    form_data[form].switches[local_index].object_mode = PRESSED;
    form_data[form].switches[local_index].object_mode == NOT_PRESSED;
}