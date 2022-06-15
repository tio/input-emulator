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
    sleep(6);
    type_string(fd, "\r", DELAY);
    type_string(fd, "# tio comes with full shell completion support\r", DELAY);
    type_string(fd, "\r", DELAY);
    sleep(2);
    type_string(fd, "tio --\t\t", DELAY);
    sleep(3);
    type_string(fd, "help\r", DELAY);
    sleep(10);
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
    type_string(fd, "tio /dev/serial/by-id/usb-Silicon_Labs_CP2105_Dual_USB_to_UART_Bridge_Controller_01093176-if01-port0\r", 25);
    sleep(3);
    type_string(fd, "\r\r", DELAY);
    type_string(fd, "# We are now connected to remote TTY terminal via a usb<->serial cable\r", DELAY + 5);
    type_string(fd, "\r", DELAY);
    sleep(3);
    type_string(fd, "uname -sniro\r", DELAY);
    sleep(3);
    type_string(fd, "ls -la\r", DELAY);
    sleep(3);
    type_string(fd, "\r", DELAY);
    type_string(fd, "# Let's try unplug and plug usb cable and see what happens..\r", DELAY);
    type_string(fd, "\r", DELAY);
    sleep(10);
    type_string(fd, "\r", DELAY);
    type_string(fd, "# tio automatically reconnected\r", DELAY);
    type_string(fd, "\r", DELAY);
    sleep(3);
    type_string(fd, "\r", DELAY);
    type_string(fd, "# In session, tio supports various useful key commands\r", DELAY);
    type_string(fd, "\r", DELAY);
    sleep(3);
    type_string(fd, "\r", DELAY);
    type_string(fd, "# Press ctrl-t ? to list available key commands\r", DELAY);
    type_string(fd, "\r", DELAY);
    sleep(3);
    key_press(fd, KEY_LEFTCTRL);
    type_string(fd, "t", DELAY);
    key_release(fd, KEY_LEFTCTRL);
    type_string(fd, "?", DELAY);
    sleep(3);
    type_string(fd, "\r\r", DELAY);
    type_string(fd, "# Show configuration by pressing ctrl-t c\r", DELAY);
    type_string(fd, "\r", DELAY);
    sleep(3);
    key_press(fd, KEY_LEFTCTRL);
    type_string(fd, "t", DELAY);
    key_release(fd, KEY_LEFTCTRL);
    type_string(fd, "c", DELAY);
    sleep(3);
    type_string(fd, "\r\r", DELAY);
    type_string(fd, "# Show the TX/RX statistics by pressing ctrl-t s\r", DELAY);
    type_string(fd, "\r", DELAY);
    sleep(3);
    key_press(fd, KEY_LEFTCTRL);
    type_string(fd, "t", DELAY);
    key_release(fd, KEY_LEFTCTRL);
    type_string(fd, "s", DELAY);
    sleep(3);
    type_string(fd, "\r\r", DELAY);
    type_string(fd, "# Show line states by pressing ctrl-t L\r", DELAY);
    type_string(fd, "\r", DELAY);
    sleep(3);
    key_press(fd, KEY_LEFTCTRL);
    type_string(fd, "t", DELAY);
    key_release(fd, KEY_LEFTCTRL);
    type_string(fd, "L", DELAY);
    sleep(3);
    type_string(fd, "\r\r", DELAY);
    type_string(fd, "# To quit simply press ctrl-t q\r", DELAY);
    type_string(fd, "\r", DELAY);
    sleep(3);
    key_press(fd, KEY_LEFTCTRL);
    type_string(fd, "t", DELAY);
    key_release(fd, KEY_LEFTCTRL);
    type_string(fd, "q", DELAY);
    sleep(3);
    type_string(fd, "\r", DELAY);
    type_string(fd, "# tio can also be configured via configuration file\r", DELAY);
    type_string(fd, "\r", DELAY);
    sleep(3);
    type_string(fd, "cat ~/.tiorc\r", DELAY);
    sleep(10);
    type_string(fd, "\r", DELAY);
    type_string(fd, "# Connect using specific sub-configuration by name\r", DELAY);
    type_string(fd, "\r", DELAY);
    sleep(3);
    type_string(fd, "tio am64-evm\r", DELAY);
    sleep(3);
    key_press(fd, KEY_LEFTCTRL);
    type_string(fd, "t", DELAY);
    key_release(fd, KEY_LEFTCTRL);
    type_string(fd, "q", DELAY);
    sleep(3);
    type_string(fd, "tio tincan\r", DELAY);
    sleep(3);
    key_press(fd, KEY_LEFTCTRL);
    type_string(fd, "t", DELAY);
    key_release(fd, KEY_LEFTCTRL);
    type_string(fd, "q", DELAY);
    sleep(6);
}
