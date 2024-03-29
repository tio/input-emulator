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
    REQ_KBD_TYPE,
    REQ_MOUSE_START,
    REQ_MOUSE_MOVE,
    REQ_MOUSE_BUTTON,
    REQ_MOUSE_BUTTONDOWN,
    REQ_MOUSE_BUTTONUP,
    REQ_MOUSE_SCROLL,
    REQ_TOUCH_START,
    REQ_TOUCH_TAP,
    REQ_STATUS,
    RSP_STATUS,
    REQ_STOP,
    RSP_STOP,
    RSP_OK,
    RSP_ERROR,
} message_type_t;

void message_server_open(void);
void message_server_close(void);
void message_client_open(void);
void message_client_mode_enable(void);
void message_client_close(void);
void message_server_listen(void (*callback)(void));
int msg_create(void **message, message_type_t type, void *payload, uint32_t payload_length);
void msg_destroy(void *message);
int msg_send(void *message);
int msg_receive(void **message);
void msg_send_rsp_ok(void);
void msg_receive_rsp_ok(void);
bool message_server_running(void);
