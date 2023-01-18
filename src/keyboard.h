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
#include <wchar.h>

int keyboard_create(void);
void keyboard_destroy(void);
bool keyboard_online(void);
const char* keyboard_sys_name(void);
void do_keyboard_keydown(void *message);
void do_keyboard_keydown_request(uint32_t key);
void do_keyboard_keyup(void *message);
void do_keyboard_keyup_request(uint32_t key);
void do_keyboard_key(void *message);
void do_keyboard_key_request(uint32_t key);
void do_keyboard_start(void *message);
void do_keyboard_start_request(void);
void do_keyboard_key_requests(const wchar_t *wc_string);
int wchar_to_key(wchar_t wc, uint32_t *key, uint32_t *modifier);
