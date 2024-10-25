/**
 * @file    neopixel.c
 * @author  Jim Herd
 * @brief   neopixel routines
 */

#include "system.h"
#include "externs.h"

#include "pico/stdlib.h"
#include "pico/sem.h"
#include "pico/binary_info.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

#include "neopixel.pio.h"


#include "neopixel.h"

#include "Pico_IO.h"

//==============================================================================
// constants
//==============================================================================

#define     NEOP_STATE_MACHINE  (0)
#define     NEOP_DMA_CHANNEL    (0)
#define     DMA_CHANNEL_MASK    (1u<<DMA_CHANNEL)
#define     NEOP_RESET_TIME_US  (60) 

//==============================================================================
// local data
//==============================================================================

struct neopixel_data_s neopixel_data[NOS_NEOPIXELS];

struct      {
    uint32_t    neopixel_pio_sm;
    uint32_t    neopixel_dma_chan;
    union {
        struct {
            uint32_t    nos_bits;
            uint32_t    neopixel_buffer[NOS_NEOPIXELS];
        } data;
        uint32_t    neopixel_cmd[NOS_NEOPIXELS+1];
    } cmd;
} neopixel_sys_buffer;

//==============================================================================
// functions
//==============================================================================

void init_neopixel_buffer(void) {

    neopixel_sys_buffer.cmd.data.nos_bits = (NOS_NEOPIXELS * 24) - 1;  
        // set to -1 as the test in the PIO is at the end of a bit transfer
    neopixel_sys_buffer.neopixel_dma_chan = NEOP_DMA_CHANNEL;
    neopixel_sys_buffer.neopixel_pio_sm   = NEOPIXEL_STATE_MACHINE;
}

void init_neopixel_sm(void) {

    uint offset = pio_add_program(NEOPIXEL_PIO_UNIT, &neopixel_program);
    neopixel_program_init(
        NEOPIXEL_PIO_UNIT, 
        NEOPIXEL_STATE_MACHINE, 
        offset,   
        NEOPIXEL_DOUT_PIN, 
        NEOPIXEL_DATA_RATE,
        NEOPIXEL_BITS_PER_UNIT 
    );
}

/**
 * @brief Set buffer pointer in DMA channel and go
 * 
 */
void trigger_neopixel_dma(void) {

    dma_channel_set_read_addr(
        neopixel_sys_buffer.neopixel_dma_chan,
        &neopixel_sys_buffer.cmd.data.neopixel_buffer[0],
        true
    );
}

/**
 * @brief Configure DMA channel
 * 
 * @param pio 
 * @param state_mach 
 */
void init_neopixel_DMA(PIO pio, uint32_t state_mach) {

    dma_channel_config channel_config = dma_channel_get_default_config(NEOP_DMA_CHANNEL);
    channel_config_set_transfer_data_size(&channel_config, DMA_SIZE_32);
	channel_config_set_read_increment(&channel_config, true);
    channel_config_set_dreq(&channel_config, DREQ_PIO0_TX0);
    dma_channel_configure(
        NEOP_DMA_CHANNEL,
        &channel_config,
        &pio0_hw->txf[0], // Write address (only need to set this once)
        NULL,             // Don't provide a read address yet
        NOS_NEOPIXELS+1,  // number of word transfers (include pixel count value as 1st word)
        false             // Don't start yet
    );
}


/**
 * @brief Put a 'grb' pixel value to the pixel buffer
 * 
 * @param pixel_no      range 0->(NOS_NEOPIXELS)
 * @param pixel_grb     RGB value (adjusted to GRB) 
 */
inline void set_pixel(uint32_t pixel_no, colours_et col) {

    neopixel_sys_buffer.cmd.data.neopixel_buffer[pixel_no] = rainbow_col[col].GRB_value << 8;  ;
}

// void put_pixel(uint32_t pixel_grb) {
//     pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
// }

uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 8)  |
            ((uint32_t) (g) << 16) |
             (uint32_t) (b);
}

void set_neopixel_on(uint8_t pixel_no, colours_et on_colour) {
    xSemaphoreTake(neopixel_data_MUTEX_access, portMAX_DELAY);
        neopixel_data[pixel_no].command = N_CMD_ON;
        neopixel_data[pixel_no].on_colour = on_colour;
    xSemaphoreGive(neopixel_data_MUTEX_access);
}

void set_neopixel_off(uint8_t pixel_no, colours_et off_colour) {
    xSemaphoreTake(neopixel_data_MUTEX_access, portMAX_DELAY);
        neopixel_data[pixel_no].command = N_CMD_OFF;
        neopixel_data[pixel_no].off_colour = off_colour;
    xSemaphoreGive(neopixel_data_MUTEX_access);
}

inline void clear_neopixel(uint8_t pixel_no) {
    set_neopixel_off(pixel_no, N_BLACK);
}

inline void clear_all_neopixels(void) {
    for (uint8_t count = 0 ; count < NOS_NEOPIXELS ; count++) {
        set_neopixel_off(count, N_BLACK);
    }
}

void set_neopixel_flash(uint8_t pixel_no, colours_et on_colour, uint32_t on_time, 
                        colours_et off_colour, uint32_t off_time) {
    xSemaphoreTake(neopixel_data_MUTEX_access, portMAX_DELAY);
        neopixel_data[pixel_no].on_colour = on_colour;
        neopixel_data[pixel_no].flash_on_time = on_time;
        neopixel_data[pixel_no].off_colour =off_colour;
        neopixel_data[pixel_no].flash_off_time = off_time;
        neopixel_data[pixel_no].command = N_CMD_FLASH;
        neopixel_data[pixel_no].state = N_FLASH_ON;
    xSemaphoreGive(neopixel_data_MUTEX_access);
}

// void set_state_neopixel(uint8_t pixel_no, NEOPIXEL_STATE_et state) 
// {
//     neopixel_data[pixel_no].state = state;
//     if (state == N_DISABLE) {
//         xSemaphoreTake(semaphore_neopixel_data, portMAX_DELAY);
//             neopixel_data[pixel_no].current_colour      = N_BLACK;
//             neopixel_data[pixel_no].current_intensity   = 0;
//         xSemaphoreGive(semaphore_neopixel_data);
//     }
// }

// void set_flash_neopixel(uint8_t pixel_no, NEOPIXEL_STATE_et state, uint8_t flash_rate)
// {
//     xSemaphoreTake(semaphore_neopixel_data, portMAX_DELAY);
//         neopixel_data[pixel_no].flags.flash_state   = state;
//         neopixel_data[pixel_no].dim_rate            = flash_rate;
//     xSemaphoreGive(semaphore_neopixel_data);
// }

// void set_dim_neopixel(uint8_t pixel_no, NEOPIXEL_STATE_et state, uint8_t dim_rate, uint8_t dim_change)
// {
//     xSemaphoreTake(semaphore_neopixel_data, portMAX_DELAY);
//         neopixel_data[pixel_no].flags.dim_state     = state;
//         neopixel_data[pixel_no].dim_rate            = dim_rate;
//         neopixel_data[pixel_no].dim_percent_change  = dim_change;
//     xSemaphoreGive(semaphore_neopixel_data);
// }
