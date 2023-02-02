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
    int32_t x_rel;
    int32_t y_rel;
} mouse_move_data_t;

typedef struct
{
    uint32_t x_max;
    uint32_t y_max;
} mouse_start_data_t;

int mouse_create(int x, int y);
void mouse_destroy(void);
bool mouse_online(void);
const char* mouse_sys_name(void);
void mouse_move(int x_rel, int y_rel);
void do_mouse_click(void *message);
void do_mouse_click_request(int button);
void do_mouse_down(void *message);
void do_mouse_down_request(int button);
void do_mouse_up(void *message);
void do_mouse_up_request(int button);
void do_mouse_scroll(void *message);
void do_mouse_scroll_request(int32_t ticks);
void do_mouse_move(void *message);
void do_mouse_move_request(int32_t x, int32_t y);
void do_mouse_start_request(uint32_t x_max, uint32_t y_max);
void do_mouse_start(void *message);
int mouse_x_max(void);
int mouse_y_max(void);
