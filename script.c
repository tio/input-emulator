/*
 * Copyright (c) 2022  Martin Lund
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

#include <unistd.h>
#include <linux/uinput.h>
#include "keyboard-simulator.h"

#define DELAY 50

void keyboard_script(int fd)
{
    type_string(fd, "\r", DELAY);
    type_string(fd, "# Tio comes with full bash completion support\r", DELAY);
    type_string(fd, "\r", DELAY);
    sleep(2);
    type_string(fd, "tio --\t\t", DELAY);
    sleep(1);
    type_string(fd, "help\r", DELAY);
    sleep(3);
    type_string(fd, "\r", DELAY);
    type_string(fd, "# Let's list available serial devices\r", DELAY);
    type_string(fd, "\r", DELAY);
    sleep(3);
    type_string(fd, "tio --list-devices\r", DELAY);
    sleep(3);
    type_string(fd, "\r", DELAY);
    type_string(fd, "# Connect to serial device\r", DELAY);
    type_string(fd, "\r", DELAY);
    sleep(3);
    type_string(fd, "tio /dev/serial/by-id/usb-Silicon_Labs_CP2105_Dual_USB_to_UART_Bridge_Controller_01093176-if00-port0\r", 25);
    sleep(3);
    key_press(fd, KEY_LEFTCTRL);
    type_key(fd, KEY_T);
    key_release(fd, KEY_LEFTCTRL);
    type_key(fd, KEY_Q);

}
