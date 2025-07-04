/**
 * @file gen4_uLCD.c
 * @author Jim Herd 
 * @brief 
 * @date 2023-11-08
 * 
 * 
 */
#include	<stdio.h>
#include	<stdint.h>
#include    <string.h>
#include	"pico/stdlib.h"

#include    "hardware/uart.h"

#include	"FreeRTOS.h"
#include    "timers.h"

#include	"externs.h"
#include	"gen4_uLCD.h"

#include	"system.h"

//==============================================================================
// Global variables
//==============================================================================

gen4_uLCD_cmd_packet_ts    uLCD_cmd;
gen4_uLCD_reply_packet_tu  uLCD_reply_data;
bool 		gen4_uLCD_detected, gen4_uLCD_test_apply;
int32_t		gen4_uLCD_current_form;

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
    gpio_put(DISPLAY_RESET_PIN, 0);
    gpio_set_dir(DISPLAY_RESET_PIN, GPIO_OUT);
    busy_wait_us(DISPLAY_WAIT_US);
    gpio_set_dir(DISPLAY_RESET_PIN, GPIO_IN);
}

//==============================================================================

error_codes_te  gen4_uLCD_init(void) 
{
error_codes_te status;

	reset_4D_display();

	gen4_uLCD_current_form = -1;
	gen4_uLCD_detected = false;
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
 * @brief Execute a ReadObject command (command 0x00)
 * 
 * @param object 	object type. List is in enum gen4_uLCD_Object_te
 * @param index     Generated by VisiGenie graphics design package
 * @return uint8_t  Error from 
 */
error_codes_te gen4_uLCD_ReadObject(uint16_t object, uint16_t index, uint32_t *result) 
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
	 uLCD_cmd.data[2] = index;

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
 * @brief Execute WriteObject command (command 0x01)
 * 
 * @param object 
 * @param index 
 * @param data 
 * @return error_codes_te 
 */
error_codes_te  gen4_uLCD_WriteObject(uint16_t object, uint16_t index, uint16_t data) 
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
	 uLCD_cmd.data[2] = index;
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
 * @brief  Execute WriteObject command (command 0x04)
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
/**
 * @brief Write ASCII string to a string object
 * 
 * @param object 
 * @param index 			String object index
 * @param text 		   		ASCII null terminated string
 * @return error_codes_te 
 */

error_codes_te    gen4_uLCD_WriteString(uint16_t index, uint8_t *text)
{
uint8_t   checksum, reply_byte, text_length;
error_codes_te   status;

	if (gen4_uLCD_test_apply == true) {
		if (gen4_uLCD_detected == false) {
			return GEN4_uLCD_NOT_DETECTED;
		}
	}
	xSemaphoreTake(gen4_uLCD_MUTEX_access, portMAX_DELAY);
	uLCD_cmd.data[0] = GEN4_uLCD_WRITE_STR;
	uLCD_cmd.data[1] = index;
	text_length = strlen(text);   // does not include terminating '\0' character
	if (text_length > MAX_GEN4_uLCD_WRITE_STR_SIZE) {
		xSemaphoreGive(gen4_uLCD_MUTEX_access);
		return GEN4_uLCD_WRITE_STR_TOO_BIG;
	}
	uLCD_cmd.cmd_length = text_length + 1 + display_cmd_info[GEN4_uLCD_WRITE_STR].length;
	uLCD_cmd.data[2] = text_length + 1;
	strcat(&uLCD_cmd.data[3], text);
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

error_codes_te change_form(int32_t new_form) 
{
error_codes_te status;

	if ((new_form > 0) && (new_form < NOS_FORMS)) {
		status = gen4_uLCD_WriteObject(GEN4_uLCD_OBJ_FORM, new_form, 0);
        if (status == OK) {
            gen4_uLCD_current_form = new_form;
        }
    } else {
		status = GEN4_uLCD_CMD_BAD_FORM_INDEX;
	}
}

/**
 * @brief Get the active form object
 * 
 * @return uint32_t -1 = no form active
 */
int32_t  get_active_form(void) {
	
	return gen4_uLCD_current_form;
}

//==============================================================================
/**
 * @brief Read state of button on the LCD display
 * 
 * @param object	Code of button type object (e.g. WINBUTTON)
 * @param index		Button index
 * @param result	Pointer to result (0 or 1)
 * @return			error code
 * 
 * @note
 
 */
error_codes_te read_LCD_button(uint32_t object, uint32_t index, uint32_t *result)
{
uint32_t button_result;
error_codes_te status;

// check if object is on the active form


// read value
    status = gen4_uLCD_ReadObject(object, index, &button_result);
	*result = button_result;
	return status;
}
//==============================================================================

int32_t detect_LCD_press(uint32_t object, uint32_t index)
{
    
}

