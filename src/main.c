/*
 * input-emulator - a scriptable input emulator
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <locale.h>
#include "signals.h"
#include "service.h"
#include "message.h"
#include "options.h"
#include "keyboard.h"
#include "touch.h"
#include "mouse.h"
#include "event.h"
#include "print.h"

void handle_message(void)
{
    void *message = NULL;
    message_header_t *header;

    /* Receive message (blocking) */
    msg_receive(&message);
    header = message;

    /* Handle incoming message request */
    switch (header->type)
    {
        case REQ_KBD_START:
            do_keyboard_start(message);
            break;

        case REQ_KBD_KEY:
            do_keyboard_key(message);
            break;

        case REQ_KBD_KEYDOWN:
            do_keyboard_keydown(message);
            break;

        case REQ_KBD_KEYUP:
            do_keyboard_keyup(message);
            break;

        case REQ_KBD_TYPE:
            do_keyboard_type(message);
            break;

        case REQ_MOUSE_START:
            do_mouse_start(message);
            break;

        case REQ_MOUSE_MOVE:
            do_mouse_move(message);
            break;

        case REQ_MOUSE_BUTTON:
            do_mouse_click(message);
            break;

        case REQ_MOUSE_BUTTONDOWN:
            do_mouse_down(message);
            break;

        case REQ_MOUSE_BUTTONUP:
            do_mouse_up(message);
            break;

        case REQ_MOUSE_SCROLL:
            do_mouse_scroll(message);
            break;

        case REQ_TOUCH_START:
            do_touch_start(message);
            break;

        case REQ_TOUCH_TAP:
            do_touch_tap(message);
            break;

        case REQ_STATUS:
            debug_printf("Received status message!\n");
            do_service_status(message);
            break;

        case REQ_STOP:
            do_service_stop(message);
            if (!devices_online())
            {
                /* No simulated input devices online - stop service */
                printf("No simulated input devices online - stopping input-emulator service!\n");
                exit(EXIT_SUCCESS);
            }
            break;

        default:
            break;
    }

    /* Destroy message */
    msg_destroy(message);
}

int main(int argc, char *argv[])
{
    /* Set default locale */
    setlocale(LC_ALL, "");

    /* Parse options */
    options_parse(argc, argv);

    /* Check if service is running on non-start commands */
    if (option.command != CMD_START)
    {
        // Check for running daemon/service
        if (!service_running())
        {
            printf("Please start service using the 'input-emulator start <device>' command\n");
            return -1;
        }

        /* Open message queue (client) */
        message_client_open();
        atexit(message_client_close);
    }

    /* Handle command */
    switch (option.command)
    {
        case CMD_START:

            if (service_running())
            {
                /* Server already running so we will act as client */

                /* Open message queue (client) */
                message_client_open();
                atexit(message_client_close);

                switch (option.device)
                {
                    case DEV_KEYBOARD:
                        do_keyboard_start_request();
                        break;

                    case DEV_MOUSE:
                        do_mouse_start_request(option.x_max, option.y_max);
                        break;

                    case DEV_TOUCH:
                        do_touch_start_request(option.x_max, option.y_max, option.slots);
                        break;

                    case DEV_ALL:
                    case DEV_NONE:
                        break;
                }
                return 0;
            }
            else
            {
                printf("Starting input-emulator service...\n");
            }

            /* Run in background */
            if (option.daemonize)
            {
                daemonize();
            }

            /* Install signal handlers */
            signal_handlers_install();

            switch (option.device)
            {
                case DEV_KEYBOARD:
                    /* Initilize keyboard input event device */
                    if (keyboard_create() == 0)
                    {
                        atexit(keyboard_destroy);
                    }
                    break;

                case DEV_MOUSE:
                    /* Initilize mouse input event device */
                    if (mouse_create(option.x_max, option.y_max) == 0)
                    {
                        atexit(mouse_destroy);
                    }
                    break;

                case DEV_TOUCH:
                    /* Initilize touch input event device */
                    if (touch_create(option.x_max, option.y_max, option.slots) == 0)
                    {
                        atexit(touch_destroy);
                    }
                    break;

                case DEV_ALL:
                case DEV_NONE:
                    break;
            }

            /* Set up message queue */
            message_server_open();
            atexit(message_server_close);

            /* Enter command handling loop */
            message_server_listen(handle_message);

            break;

        case CMD_KBD:

            switch (option.kbd_action)
            {
                case KBD_KEY:
                    do_keyboard_key_request(option.key);
                    break;

                case KBD_KEYDOWN:
                    do_keyboard_keydown_request(option.key);
                    break;

                case KBD_KEYUP:
                    do_keyboard_keyup_request(option.key);
                    break;

                case KBD_TYPE:
                    do_keyboard_type_request(option.wc_string);
                    break;

                case KBD_NONE:
                    break;
            }
            break;

        case CMD_MOUSE:

            switch (option.mouse_action)
            {
                case MOUSE_MOVE:
                    do_mouse_move_request(option.x, option.y);
                    break;

                case MOUSE_BUTTON:
                    do_mouse_click_request(option.button);
                    break;

                case MOUSE_BUTTONDOWN:
                    do_mouse_down_request(option.button);
                    break;

                case MOUSE_BUTTONUP:
                    do_mouse_up_request(option.button);
                    break;

                case MOUSE_SCROLL:
                    do_mouse_scroll_request(option.ticks);
                    break;

                case MOUSE_NONE:
                    break;
            }
            break;

        case CMD_TOUCH:

            switch (option.touch_action)
            {
                case TOUCH_TAP:
                    do_touch_tap_request(option.x, option.y, option.duration);
                    break;

                case TOUCH_NONE:
                    break;
            }
            break;

        case CMD_STATUS:
            do_service_status_request();
            break;

        case CMD_STOP:
            switch (option.device)
            {
                case DEV_KEYBOARD:
                    do_service_stop_request(DEV_KEYBOARD);
                    break;

                case DEV_MOUSE:
                    do_service_stop_request(DEV_MOUSE);
                    break;

                case DEV_TOUCH:
                    do_service_stop_request(DEV_TOUCH);
                    break;

                case DEV_ALL:
                    do_service_stop_request(DEV_ALL);
                    break;


                case DEV_NONE:
                    break;
            }

        default:
            break;
    }

    return 0;
}
