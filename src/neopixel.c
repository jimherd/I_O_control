/**
 * @file    neopixel.c
 * @author  Jim Herd
 * @brief   neopixel routines
 */

#include "system.h"
//#include "common.h"
#include "externs.h"

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

#include "neopixel.h"

#include "Pico_IO.h"

//
// local data
//
struct neopixel_data_s neopixel_data[NOS_NEOPIXELS];

void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 8)  |
            ((uint32_t) (g) << 16) |
             (uint32_t) (b);
}

void set_neopixel_on(uint8_t pixel_no, colours_et on_colour)
{
    xSemaphoreTake(semaphore_neopixel_data, portMAX_DELAY);
        neopixel_data[pixel_no].command = N_CMD_ON;
        neopixel_data[pixel_no].current_colour = on_colour;
    xSemaphoreGive(semaphore_neopixel_data);
}

void set_neopixel_off(uint8_t pixel_no, colours_et off_colour)
{
    xSemaphoreTake(semaphore_neopixel_data, portMAX_DELAY);
        neopixel_data[pixel_no].command = N_CMD_OFF;
        neopixel_data[pixel_no].current_colour = off_colour;
    xSemaphoreGive(semaphore_neopixel_data);
}

inline void clear_neopixel(uint8_t pixel_no)
{
    set_neopixel_off(pixel_no, N_BLACK);
}

inline void clear_all_neopixels(void)
{
    for (uint8_t count = 0 ; count < NOS_NEOPIXELS ; count++) {
        set_neopixel_off(count, N_BLACK);
    }
}

void set_neopixel_flash(uint8_t pixel_no, colours_et on_colour, uint32_t on_time, colours_et off_colour, uint32_t off_time)
{
    xSemaphoreTake(semaphore_neopixel_data, portMAX_DELAY);
        neopixel_data[pixel_no].on_colour = on_colour;
        neopixel_data[pixel_no].flash_on_time = on_time;
        neopixel_data[pixel_no].off_colour =off_colour;
        neopixel_data[pixel_no].flash_off_time = off_time;
        neopixel_data[pixel_no].command = N_CMD_FLASH;
        neopixel_data[pixel_no].state = N_FLASH_ON;
    xSemaphoreGive(semaphore_neopixel_data);
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
