
#ifndef __GEN4_uLCD_H__
#define __GEN4_uLCD_H__

//#include    "system.h"

// // Do not modify current values. Recommended settings.
// #define DISPLAY_TIMEOUT         2000
// #define AUTO_PING_CYCLE         1250

//==============================================================================
// Function prototypes
//==============================================================================

error_codes_te  gen4_uLCD_init(void);
void              uart1_sys_init(void);
void              reset_4D_display(void);
error_codes_te    gen4_uLCD_ReadObject(uint16_t object, uint16_t index, uint32_t *result);
error_codes_te    gen4_uLCD_WriteObject(uint16_t object, uint16_t index, uint16_t data);
error_codes_te    gen4_uLCD_WriteContrast(uint8_t value);
error_codes_te    gen4_uLCD_WriteString(uint16_t index, uint8_t *text);
void     flush_RX_fifo(uart_inst_t *uart);
int32_t  get_active_form(void);
error_codes_te    change_form(int32_t new_form);
error_codes_te    read_LCD_button(uint32_t object, uint32_t index, uint32_t *result);

#endif  /* __GEN4_uLCD_H__ */
