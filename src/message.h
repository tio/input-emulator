/*
 * input-emulator - a simple input emulator
 *
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

typedef struct __attribute__((__packed__))
{
    uint8_t type;
    uint32_t payload_length;
} message_header_t;

typedef enum
{
    REQ_KBD_START,
    REQ_KBD_KEY,
    REQ_KBD_KEYDOWN,
    REQ_KBD_KEYUP,
    REQ_MOUSE_START,
    REQ_MOUSE_MOVE,
    REQ_MOUSE_CLICK,
    REQ_MOUSE_DOWN,
    REQ_MOUSE_UP,
    REQ_TOUCH_START,
    REQ_TOUCH_TAP,
    REQ_STATUS,
    RSP_STATUS,
    REQ_STOP,
    RSP_STOP,
} message_type_t;

void message_queue_server_open(void);
void message_queue_server_close(void);
void message_queue_client_open(void);
void message_queue_client_close(void);
int msg_create(void **message, message_type_t type, void *payload, uint32_t payload_length);
void msg_destroy(void *message);
int msg_send(void *message);
int msg_receive(void **message);
