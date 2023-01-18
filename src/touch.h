/*
 * Copyright (C) 2023  DEIF A/S
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint32_t x;
    uint32_t y;
    uint32_t duration;
} touch_tap_data_t;

typedef struct
{
    uint32_t x_max;
    uint32_t y_max;
    uint8_t slots;
} touch_start_data_t;

int touch_create(int x, int y, int slots);
void touch_destroy(void);
bool touch_online(void);
const char* touch_sys_name(void);
void touch_tap(int x, int y, int duration);
void do_touch_tap(void *message);
void do_touch_tap_request(uint32_t x, uint32_t y, uint32_t duration);
void do_touch_start(void *message);
void do_touch_start_request(uint32_t x_max, uint32_t y_max, uint8_t slots);

