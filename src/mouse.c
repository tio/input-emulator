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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/uinput.h>
#include <errno.h>
#include "options.h"
#include "message.h"
#include "event.h"
#include "mouse.h"
#include "service.h"
#include "print.h"
#include "misc.h"

static int mouse_fd = -1;
static char sys_name[SYS_NAME_LENGTH_MAX];
static int mouse_config_x_max;
static int mouse_config_y_max;

int mouse_x_max(void)
{
    return mouse_config_x_max;
}

int mouse_y_max(void)
{
    return mouse_config_y_max;
}

void mouse_move(int x_rel, int y_rel)
{
    /* Do nothing if no device */
    if (mouse_fd < 0)
    {
        return;
    }

    debug_printf("Mouse move %d,%d\n", x_rel, y_rel);

    // Move mouse absolute
    // emit(mouse_fd, EV_ABS, ABS_X, x);
    // emit(mouse_fd, EV_ABS, ABS_Y, y);
    // emit(mouse_fd, EV_SYN, SYN_REPORT, 0);

    // Move mouse relative
    emit(mouse_fd, EV_REL, REL_X, x_rel);
    emit(mouse_fd, EV_REL, REL_Y, y_rel);
    emit(mouse_fd, EV_SYN, SYN_REPORT, 0);
}

void mouse_press(int button)
{
    /* Do nothing if no device */
    if (mouse_fd < 0)
    {
        return;
    }

    // Press button
    emit(mouse_fd, EV_KEY, button, 1);
    emit(mouse_fd, EV_SYN, SYN_REPORT, 0);
}

void mouse_release(int button)
{
    /* Do nothing if no device */
    if (mouse_fd < 0)
    {
        return;
    }

    // Press button
    emit(mouse_fd, EV_KEY, button, 0);
    emit(mouse_fd, EV_SYN, SYN_REPORT, 0);
}

void mouse_click(int button)
{
    /* Do nothing if no device */
    if (mouse_fd < 0)
    {
        return;
    }

    debug_printf("Mouse click 0x%x\n", button);

    mouse_press(button);
    usleep(1000);
    mouse_release(button);
}

void mouse_scroll(int32_t ticks)
{
    /* Do nothing if no device */
    if (mouse_fd < 0)
    {
        return;
    }

    // Scroll wheel number of ticks
    emit(mouse_fd, EV_REL, REL_WHEEL, ticks);
    emit(mouse_fd, EV_SYN, SYN_REPORT, 0);
}

int mouse_create(int x_max, int y_max)
{
    static struct uinput_setup usetup;
    static struct uinput_abs_setup abs_setup;

    mouse_config_x_max = x_max;
    mouse_config_y_max = y_max;

    if (mouse_fd >= 0)
    {
        /* Mouse already started */
        return -1;
    }

    mouse_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (mouse_fd < 0)
    {
        error_printf("Could not open /dev/uinput (%s)\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Enable button events */
    do_ioctl(mouse_fd, UI_SET_EVBIT, EV_KEY);
    do_ioctl(mouse_fd, UI_SET_KEYBIT, BTN_LEFT);
    do_ioctl(mouse_fd, UI_SET_KEYBIT, BTN_MIDDLE);
    do_ioctl(mouse_fd, UI_SET_KEYBIT, BTN_RIGHT);
    do_ioctl(mouse_fd, UI_SET_KEYBIT, BTN_SIDE);
    do_ioctl(mouse_fd, UI_SET_KEYBIT, BTN_EXTRA);

    /* Enable relative movement events */
    do_ioctl(mouse_fd, UI_SET_EVBIT, EV_REL);
    do_ioctl(mouse_fd, UI_SET_RELBIT, REL_X);
    do_ioctl(mouse_fd, UI_SET_RELBIT, REL_Y);

    /* Enable absolute movement events */
    do_ioctl(mouse_fd, UI_SET_EVBIT, EV_ABS);
    do_ioctl(mouse_fd, UI_SET_ABSBIT, ABS_X);
    do_ioctl(mouse_fd, UI_SET_ABSBIT, ABS_Y);

    /* Set up mouse properties (resolution) */
    abs_setup.code = ABS_X;
    abs_setup.absinfo.minimum = 0;
    abs_setup.absinfo.maximum = x_max;
    do_ioctl(mouse_fd, UI_ABS_SETUP, &abs_setup);

    abs_setup.code = ABS_Y;
    abs_setup.absinfo.minimum = 0;
    abs_setup.absinfo.maximum = y_max;
    do_ioctl(mouse_fd, UI_ABS_SETUP, &abs_setup);

    /* Set up device */
    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1111;
    usetup.id.product = 0x1111;
    usetup.id.version = 1;
    strcpy(usetup.name, "Simulated mouse");
    do_ioctl(mouse_fd, UI_DEV_SETUP, &usetup);

    /* Create device */
    do_ioctl(mouse_fd, UI_DEV_CREATE);

    /*
     * On UI_DEV_CREATE the kernel will create the device node for this
     * device. We are inserting a pause here so that userspace has time
     * to detect, initialize the new device, and can start listening to
     * the event, otherwise it will not notice the event we are about
     * to send. This pause is only needed in our example code!
     */
    sleep(1);

    device_ref_count++;

    debug_printf("Created mouse input device with x-max=%d y-max=%d\n", x_max, y_max);

    /* Save sys name */
    do_ioctl(mouse_fd, UI_GET_SYSNAME(50), sys_name);

    return 0;
}

void mouse_destroy(void)
{
    /*
     * Give userspace some time to read the events before we destroy the
     * device with UI_DEV_DESTROY.
     */
    sleep(1);

    if (mouse_fd < 0)
    {
        return;
    }

    debug_printf("Destroying mouse input device\n");

    do_ioctl(mouse_fd, UI_DEV_DESTROY);
    close(mouse_fd);

    mouse_fd = -1;

    sys_name[0] = 0;

    device_ref_count--;
}

const char* mouse_sys_name(void)
{
    return sys_name;
}

bool mouse_online(void)
{
    if (mouse_fd >= 0)
    {
        return true;
    }

    return false;
}

void do_mouse_click(void *message)
{
    message_header_t *header = message;
    int *button = message + sizeof(message_header_t);

    if (header->payload_length != sizeof(int))
    {
        warning_printf("Invalid payload length");
        return;
    }

    mouse_click(*button);

    msg_send_rsp_ok();
}

void do_mouse_click_request(int button)
{
    void *message = NULL;

    msg_create(&message, REQ_MOUSE_BUTTON, &button, sizeof(button));
    msg_send(message);
    msg_destroy(message);

    msg_receive_rsp_ok();
}

void do_mouse_scroll(void *message)
{
    message_header_t *header = message;
    int32_t *ticks = message + sizeof(message_header_t);

    if (header->payload_length != sizeof(int32_t))
    {
        warning_printf("Invalid payload length");
        return;
    }

    mouse_scroll(*ticks);

    msg_send_rsp_ok();
}

void do_mouse_scroll_request(int32_t ticks)
{
    void *message = NULL;

    msg_create(&message, REQ_MOUSE_SCROLL, &ticks, sizeof(ticks));
    msg_send(message);
    msg_destroy(message);

    msg_receive_rsp_ok();
}

void do_mouse_down(void *message)
{
    message_header_t *header = message;
    int *button = message + sizeof(message_header_t);

    if (header->payload_length != sizeof(int))
    {
        warning_printf("Invalid payload length");
        return;
    }

    mouse_press(*button);

    msg_send_rsp_ok();
}

void do_mouse_down_request(int button)
{
    void *message = NULL;

    msg_create(&message, REQ_MOUSE_BUTTONDOWN, &button, sizeof(button));
    msg_send(message);
    msg_destroy(message);

    msg_receive_rsp_ok();
}

void do_mouse_up(void *message)
{
    message_header_t *header = message;
    int *button = message + sizeof(message_header_t);

    if (header->payload_length != sizeof(int))
    {
        warning_printf("Invalid payload length");
        return;
    }

    mouse_release(*button);

    msg_send_rsp_ok();
}

void do_mouse_up_request(int button)
{
    void *message = NULL;

    msg_create(&message, REQ_MOUSE_BUTTONUP, &button, sizeof(button));
    msg_send(message);
    msg_destroy(message);

    msg_receive_rsp_ok();
}

void do_mouse_move(void *message)
{
    message_header_t *header = message;
    mouse_move_data_t *move = message + sizeof(message_header_t);

    if (header->payload_length != sizeof(mouse_move_data_t))
    {
        warning_printf("Invalid payload length");
        return;
    }

    mouse_move(move->x, move->y);

    msg_send_rsp_ok();
}

void do_mouse_move_request(int32_t x, int32_t y)
{
    void *message = NULL;
    mouse_move_data_t mouse_move_data;

    mouse_move_data.x = x;
    mouse_move_data.y = y;

    msg_create(&message, REQ_MOUSE_MOVE, &mouse_move_data, sizeof(mouse_move_data_t));
    msg_send(message);
    msg_destroy(message);

    msg_receive_rsp_ok();
}

void do_mouse_start(void *message)
{
    message_header_t *header = message;
    mouse_start_data_t *data = message + sizeof(message_header_t);

    if (header->payload_length != sizeof(mouse_start_data_t))
    {
        warning_printf("Warning: Invalid payload length\n");
        return;
    }

    mouse_create(data->x_max, data->y_max);

    msg_send_rsp_ok();
}

void do_mouse_start_request(uint32_t x_max, uint32_t y_max)
{
    void *message = NULL;
    mouse_start_data_t data;

    debug_printf("Sending mouse start message!\n");

    data.x_max = x_max;
    data.y_max = y_max;

    msg_create(&message, REQ_MOUSE_START, &data, sizeof(data));
    msg_send(message);
    msg_destroy(message);

    msg_receive_rsp_ok();
}

