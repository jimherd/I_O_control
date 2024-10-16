/**
 * @file    neopixel.h
 * @author  Jim Herd
 * @brief   neopixel data
 * @date    2022-02-26
 */
#ifndef __NEOPIXEL_H__
#define __NEOPIXEL_H__

#include <stdio.h>
#include "system.h"

void put_pixel(uint32_t pixel_grb);
uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b);

void set_neopixel_on(uint8_t pixel_no, colours_et on_colour);
void set_neopixel_off(uint8_t pixel_no, colours_et off_colour);
void set_neopixel_flash(uint8_t pixel_no, colours_et on_colour, uint32_t on_time, colours_et off_colour, uint32_t off_time);

void clear_neopixel(uint8_t pixel_no);
void clear_all_neopixels(void);
void set_state_neopixel(uint8_t pixel_no, NEOPIXEL_STATE_et state);
void set_flash_neopixel(uint8_t pixel_no, NEOPIXEL_STATE_et state, uint8_t flash_rate);
void set_dim_neopixel(uint8_t pixel_no, NEOPIXEL_STATE_et state, uint8_t dim_rate, uint8_t dim_change);

#endif