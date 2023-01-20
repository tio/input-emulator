/*
 * Copyright (C) 2023  Martin Lund
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
#include <string.h>
#include <getopt.h>
#include <uchar.h>
#include <wchar.h>
#include <linux/uinput.h>
#include <linux/input-event-codes.h>
#include "options.h"
#include "config.h"
#include "print.h"
#include "misc.h"
#include "keyboard.h"

option_t option =
{
    .command = CMD_NONE,
    .device = DEV_NONE,
    .x_max = 1024,
    .y_max = 768,
    .slots = 4,
    .kbd_action = KBD_NONE,
    .string = NULL,
    .key = 0,
    .type_delay = 40,
    .mouse_action = MOUSE_NONE,
    .ticks = 0,
    .button = -1,
    .touch_action = TOUCH_NONE,
    .x = -1,
    .y = -1,
    .duration = 15,
    .daemonize = true,
    .wc_string = NULL,
};

void options_help_print(void)
{
    printf("Usage: input-emulator [--version] [--help] <command> [<arguments>]\n");
    printf("\n");
    printf("  -v, --version                      Display version\n");
    printf("  -h, --help                         Display help\n");
    printf("\n");
    printf("Available commands:\n");
    printf("  start [<options>] kbd|mouse|touch  Create virtual input device\n");
    printf("  kbd <action> [args]                Do keyboard action\n");
    printf("  mouse <action> [args]              Do mouse action\n");
    printf("  touch <action> [args]              Do touch action\n");
    printf("  status                             Show status of virtual input devices\n");
    printf("  stop kbd|mouse|touch|all           Destroy virtual input device\n");
    printf("\n");
    printf("Start options:\n");
    printf("  -x, --x-max <int>                  Maximum x-coordinate (only for mouse and touch)\n");
    printf("  -y, --y-max <int>                  Maximum y-coordinate (only for mouse and touch)\n");
    printf("  -s, --slots <int>                  Maximum number of slots (fingers) recognized (only for touch)\n");
    printf("  -n, --no-daemonize                 Run in foreground\n");
    printf("\n");
    printf("Keyboard actions:\n");
    printf("  type '<string>'                    Type string\n");
    printf("  key <key>                          Stroke key\n");
    printf("  keydown <key>                      Press and hold key\n");
    printf("  keyup <key>                        Release key\n");
    printf("\n");
    printf("Mouse actions:\n");
    printf("  move <x> <y>                       Move mouse x,y relative\n");
    printf("  click left|middle|right            Click mouse button\n");
    printf("  down left|middle|right             Push mouse button down\n");
    printf("  up left|middle|right               Release mouse button\n");
    printf("\n");
    printf("Touch actions:\n");
    printf("  tap <x> <y>                        Tap at x,y coordinate\n");
    printf("\n");
}

void options_version_print(void)
{
    printf("input-emulator v%s\n", VERSION);
}

void options_parse(int argc, char *argv[])
{
    int c;

    /* Print help if no arguments */
    if (argc == 1)
    {
        options_help_print();
        exit(EXIT_SUCCESS);
    }

    /* getopt_long stores the option index here */
    int option_index = 0;

    /* Skip ahead past command */
    optind = 2;

    if (strcmp(argv[1], "start") == 0)
    {
        option.command = CMD_START;

        static struct option long_options[] =
        {
            {"x-max",          required_argument, 0, 'x'},
            {"y-max",          required_argument, 0, 'y'},
            {"slots",          required_argument, 0, 's'},
            {"no-daemonize",   no_argument,       0, 'n'},
            {0,                0,                 0,  0 }
        };

        do
        {
            /* Parse start options */
            c = getopt_long(argc, argv, "x:y:s:n", long_options, &option_index);

            switch (c)
            {
                case 'x':
                    option.x_max = atoi(optarg);
                    break;

                case 'y':
                    option.y_max = atoi(optarg);
                    break;

                case 's':
                    option.slots = atoi(optarg);
                    break;

                case 'n':
                    option.daemonize = false;
                    break;

                case '?':
                    exit(EXIT_FAILURE);
            }
        } while (c != -1);
    }
    else if (strcmp(argv[1], "stop") == 0)
    {
        option.command = CMD_STOP;
    }
    else if (strcmp(argv[1], "kbd") == 0)
    {
        option.command = CMD_KBD;

        if (optind != argc)
        {
            if (strcmp(argv[optind], "type") == 0)
            {
                debug_printf("type!\n");
                option.kbd_action = KBD_TYPE;
                optind++;
                if (optind != argc)
                {
                    option.string = strdup(argv[optind]);
                    option.wc_string = convert_mbs_to_wcs(argv[optind]);

                    optind++;
                    debug_printf("string = '%s'\n", option.string);
                }
            }
            else if (strcmp(argv[optind], "key") == 0)
            {
                debug_printf("key!\n");
                option.kbd_action = KBD_KEY;
                optind++;
                if (optind != argc)
                {
                    option.wc_string = convert_mbs_to_wcs(argv[optind]);
                    wchar_or_alias_to_key(option.wc_string, &option.key);

                    optind++;
                }
            }
            else if (strcmp(argv[optind], "keydown") == 0)
            {
                debug_printf("keydown!\n");
                option.kbd_action = KBD_KEYDOWN;
                optind++;
                if (optind != argc)
                {
                    option.wc_string = convert_mbs_to_wcs(argv[optind]);
                    wchar_or_alias_to_key(option.wc_string, &option.key);

                    optind++;
                }
            }
            else if (strcmp(argv[optind], "keyup") == 0)
            {
                debug_printf("keyup!\n");
                option.kbd_action = KBD_KEYUP;
                optind++;
                if (optind != argc)
                {
                    option.wc_string = convert_mbs_to_wcs(argv[optind]);
                    wchar_or_alias_to_key(option.wc_string, &option.key);

                    optind++;
                }
            }
        }
    }
    else if (strcmp(argv[1], "mouse") == 0)
    {
        option.command = CMD_MOUSE;

    }
    else if (strcmp(argv[1], "touch") == 0)
    {
        option.command = CMD_TOUCH;

    }
    else if (strcmp(argv[1], "status") == 0)
    {
        option.command = CMD_STATUS;

    }
    else
    {
        // No command provided so we restore index
        optind = 1;

        static struct option long_options[] =
        {
            {"version",        no_argument,       0, 'v'},
            {"help",           no_argument,       0, 'h'},
            {0,                0,                 0,  0 }
        };

        do
        {
            /* Parse options */
            c = getopt_long(argc, argv, "vh", long_options, &option_index);

            switch (c)
            {
                case 'v':
                    options_version_print();
                    exit(EXIT_SUCCESS);

                case 'h':
                    options_help_print();
                    exit(EXIT_SUCCESS);

                case '?':
                    exit(EXIT_FAILURE);
            }
        } while (c != -1);
    }

    if ((option.command == CMD_NONE) && (optind != argc))
    {
        error_printf("Unknown command\n");
        exit(EXIT_FAILURE);
    }

    if ((option.command == CMD_START) || (option.command == CMD_STOP))
    {
        if (optind != argc)
        {
            if (strcmp(argv[optind],"kbd") == 0)
            {
                option.device = DEV_KEYBOARD;
                optind++;
            }
            else if (strcmp(argv[optind],"mouse") == 0)
            {
                option.device = DEV_MOUSE;
                optind++;
            }
            else if (strcmp(argv[optind],"touch") == 0)
            {
                option.device = DEV_TOUCH;
                optind++;
            }

            if ((option.command == CMD_STOP) && (strcmp(argv[optind], "all") == 0))
            {
                option.device = DEV_ALL;
                optind++;
            }
        }

        if (option.device == DEV_NONE)
        {
            if (option.command == CMD_START)
            {
                error_printf("Please specify which device (kbd, mouse, touch) to start\n");
            }
            else
            {
                error_printf("Please specify which device (kbd, mouse, touch, all) to stop\n");
            }
            exit(EXIT_FAILURE);
        }
    }

    if (option.command == CMD_KBD)
    {
        if (option.kbd_action == KBD_NONE)
        {
            error_printf("Please specify kbd <action>\n");
            exit(EXIT_FAILURE);
        }

        if ((option.kbd_action == KBD_TYPE) && (option.string == NULL))
        {
            error_printf("Please specify type <string>\n");
            exit(EXIT_FAILURE);
        }

        if ((option.kbd_action == KBD_KEY) && (option.key == 0))
        {
            error_printf("Please specify key <key>\n");
            exit(EXIT_FAILURE);
        }
    }

    if (option.command == CMD_TOUCH)
    {
        if (optind != argc)
        {
            if (strcmp(argv[optind], "tap") == 0)
            {
                option.touch_action = TOUCH_TAP;
                optind++;
                if (optind != argc)
                {
                    option.x = atoi(argv[optind]);
                    optind++;
                    if (optind != argc)
                    {
                        option.y = atoi(argv[optind]);
                        optind++;
                    }
                }
            }
        }

        if (option.touch_action == TOUCH_TAP)
        {
            if ((option.x == -1) || (option.y == -1))
            {
                error_printf("Please specify tap <x> <y>\n");
                exit(EXIT_FAILURE);
            }
        }

        if (option.touch_action == TOUCH_NONE)
        {
            error_printf("Please specify touch <action>\n");
            exit(EXIT_FAILURE);
        }
    }

    if (option.command == CMD_MOUSE)
    {
        if (optind != argc)
        {
            if (strcmp(argv[optind], "move") == 0)
            {
                option.mouse_action = MOUSE_MOVE;
                optind++;
                if (optind != argc)
                {
                    option.x = atoi(argv[optind]);
                    optind++;
                    if (optind != argc)
                    {
                        option.y = atoi(argv[optind]);
                        optind++;
                    }
                }
            }
            else if (strcmp(argv[optind], "click") == 0)
            {
                option.mouse_action = MOUSE_CLICK;
                optind++;
                if (optind != argc)
                {
                    if (strcmp(argv[optind],"left") == 0)
                    {
                        option.button = BTN_LEFT;
                    }
                    else if (strcmp(argv[optind],"middle") == 0)
                    {
                        option.button = BTN_MIDDLE;
                    }
                    else if (strcmp(argv[optind],"right") == 0)
                    {
                        option.button = BTN_RIGHT;
                    }
                    optind++;
                }
            }
            else if (strcmp(argv[optind], "down") == 0)
            {
                option.mouse_action = MOUSE_DOWN;
                optind++;
                if (optind != argc)
                {
                    if (strcmp(argv[optind],"left") == 0)
                    {
                        option.button = BTN_LEFT;
                    }
                    else if (strcmp(argv[optind],"middle") == 0)
                    {
                        option.button = BTN_MIDDLE;
                    }
                    else if (strcmp(argv[optind],"right") == 0)
                    {
                        option.button = BTN_RIGHT;
                    }
                    optind++;
                }
            }
            else if (strcmp(argv[optind], "up") == 0)
            {
                option.mouse_action = MOUSE_UP;
                optind++;
                if (optind != argc)
                {
                    if (strcmp(argv[optind],"left") == 0)
                    {
                        option.button = BTN_LEFT;
                    }
                    else if (strcmp(argv[optind],"middle") == 0)
                    {
                        option.button = BTN_MIDDLE;
                    }
                    else if (strcmp(argv[optind],"right") == 0)
                    {
                        option.button = BTN_RIGHT;
                    }
                    optind++;
                }
            }
            else if (strcmp(argv[optind], "scroll") == 0)
            {
                option.mouse_action = MOUSE_SCROLL;
                optind++;
                if (optind != argc)
                {
                    option.ticks = atoi(argv[optind]);
                    optind++;
                }
            }
        }

        if (option.mouse_action == MOUSE_CLICK)
        {
            if (option.button == -1)
            {
                error_printf("Please specify click <button>\n");
                exit(EXIT_FAILURE);
            }
        }

        if (option.mouse_action == MOUSE_MOVE)
        {
            if ((option.x == -1) || (option.y == -1))
            {
                error_printf("Please specify move <x> <y>\n");
                exit(EXIT_FAILURE);
            }
        }

        if (option.mouse_action == MOUSE_NONE)
        {
            error_printf("Please specify mouse <action>\n");
            exit(EXIT_FAILURE);
        }
    }


    /* Print any unknown arguments */
    if (optind < argc)
    {
        error_printf_raw("Unknown arguments: ");
        while (optind < argc)
            error_printf_raw("%s ", argv[optind++]);
        error_printf_raw("\n");
        exit(EXIT_FAILURE);
    }
}
