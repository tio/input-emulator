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
#include <stdlib.h>
#include <stdbool.h>
#include <uchar.h>

typedef enum
{
    CMD_START,
    CMD_STOP,
    CMD_KBD,
    CMD_MOUSE,
    CMD_TOUCH,
    CMD_STATUS,
    CMD_NONE
} command_t;

typedef enum
{
    DEV_KEYBOARD,
    DEV_MOUSE,
    DEV_TOUCH,
    DEV_NONE,
} device_t;

typedef enum
{
    KBD_KEY,
    KBD_KEYDOWN,
    KBD_KEYUP,
    KBD_TYPE,
    KBD_NONE,
} kbd_action_t;

typedef enum
{
    MOUSE_MOVE,
    MOUSE_CLICK,
    MOUSE_DOWN,
    MOUSE_UP,
    MOUSE_NONE,
} mouse_action_t;

typedef enum
{
    TOUCH_TAP,
    TOUCH_NONE,
} touch_action_t;

typedef struct
{
    command_t command;
    device_t device;
    uint32_t x_max;
    uint32_t y_max;
    int slots;
    kbd_action_t kbd_action;
    char *string;
    wchar_t *wc_string;
    uint32_t key;
    uint32_t type_delay;
    mouse_action_t mouse_action;
    int button;
    touch_action_t touch_action;
    int32_t x;
    int32_t y;
    uint32_t duration;
    bool daemonize;
} option_t;

extern option_t option;

void options_help_print(void);
void options_parse(int argc, char *argv[]);
