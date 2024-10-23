/**
 * @file    neopixel.h
 * @author  Jim Herd
 * @brief   neopixel data
 * @date    2022-02-26
 */
#ifndef __NEOPIXEL_H__
#define __NEOPIXEL_H__

#include <stdio.h>

#include "hardware/pio.h"

#include "system.h"

// void put_pixel(uint32_t pixel_grb);
void load_pixel(uint32_t pixel_no, uint32_t pixel_grb);
uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b);

void set_neopixel_on(uint8_t pixel_no, colours_et on_colour);
void set_neopixel_off(uint8_t pixel_no, colours_et off_colour);
void set_neopixel_flash(uint8_t pixel_no, colours_et on_colour, uint32_t on_time, colours_et off_colour, uint32_t off_time);

void clear_neopixel(uint8_t pixel_no);
void clear_all_neopixels(void);

void init_neopixel_buffer(void);
void init_neopixel_sm(void);
void neopixel_DMA_init(PIO pio, uint32_t state_mach);
void trigger_neopixel_dma(void);

#endif