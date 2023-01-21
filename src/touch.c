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
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/uinput.h>
#include <errno.h>
#include "touch.h"
#include "event.h"
#include "filesystem.h"
#include "options.h"
#include "message.h"
#include "service.h"
#include "print.h"
#include "misc.h"

static int touch_fd = -1;
static uint32_t touch_id = 0;
static char sys_name[SYS_NAME_LENGTH_MAX];

bool touch_online(void)
{
    if (touch_fd >= 0)
    {
        return true;
    }

    return false;
}

void touch_tap(int x, int y, int duration)
{
    /* Do nothing if no device */
    if (touch_fd < 0)
    {
        return;
    }

    // One touch tap
    emit(touch_fd, EV_ABS, ABS_MT_TRACKING_ID, touch_id++);
    emit(touch_fd, EV_ABS, ABS_MT_POSITION_X, x);
    emit(touch_fd, EV_ABS, ABS_MT_POSITION_Y, y);
    emit(touch_fd, EV_KEY, BTN_TOUCH, 1);
    emit(touch_fd, EV_ABS, ABS_X, x);
    emit(touch_fd, EV_ABS, ABS_Y, y);
    emit(touch_fd, EV_SYN, SYN_REPORT, 0);

    usleep(duration*1000);

    emit(touch_fd, EV_ABS, ABS_MT_TRACKING_ID, -1);
    emit(touch_fd, EV_KEY, BTN_TOUCH, 0);
    emit(touch_fd, EV_SYN, SYN_REPORT, 0);
}

int touch_create(int x_max, int y_max, int slots)
{
    static struct uinput_setup usetup;
    static struct uinput_abs_setup abs_setup;

    if (touch_fd >= 0)
    {
        /* Touch already started */
        return -1;
    }

    touch_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (touch_fd < 0)
    {
        error_printf("Could not open /dev/uinput (%s)\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Enable absolute events (touchscreen) */
    do_ioctl(touch_fd, UI_SET_EVBIT, EV_ABS);

    do_ioctl(touch_fd, UI_SET_EVBIT, EV_KEY);
    do_ioctl(touch_fd, UI_SET_KEYBIT, BTN_TOUCH);

    do_ioctl(touch_fd, UI_SET_ABSBIT, ABS_MT_SLOT);
    do_ioctl(touch_fd, UI_SET_ABSBIT, ABS_MT_POSITION_X);
    do_ioctl(touch_fd, UI_SET_ABSBIT, ABS_MT_POSITION_Y);
    do_ioctl(touch_fd, UI_SET_ABSBIT, ABS_MT_TRACKING_ID);

    /* Set up touchscreen properties */
    abs_setup.code = ABS_X;
    abs_setup.absinfo.minimum = 0;
    abs_setup.absinfo.maximum = x_max;
    do_ioctl(touch_fd, UI_ABS_SETUP, &abs_setup);

    abs_setup.code = ABS_Y;
    abs_setup.absinfo.minimum = 0;
    abs_setup.absinfo.maximum = y_max;
    do_ioctl(touch_fd, UI_ABS_SETUP, &abs_setup);

    abs_setup.code = ABS_MT_POSITION_X;
    abs_setup.absinfo.minimum = 0;
    abs_setup.absinfo.maximum = x_max;
    do_ioctl(touch_fd, UI_ABS_SETUP, &abs_setup);

    abs_setup.code = ABS_MT_POSITION_Y;
    abs_setup.absinfo.minimum = 0;
    abs_setup.absinfo.maximum = y_max;
    do_ioctl(touch_fd, UI_ABS_SETUP, &abs_setup);

    abs_setup.code = ABS_MT_SLOT;
    abs_setup.absinfo.minimum = 0;
    abs_setup.absinfo.maximum = slots;
    do_ioctl(touch_fd, UI_ABS_SETUP, &abs_setup);

    /* Set up device */
    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1234; /* sample vendor */
    usetup.id.product = 0x5678; /* sample product */
    strcpy(usetup.name, "Simulated touchscreen");
    do_ioctl(touch_fd, UI_DEV_SETUP, &usetup);

    /* Create device */
    do_ioctl(touch_fd, UI_DEV_CREATE);

    /*
     * On UI_DEV_CREATE the kernel will create the device node for this
     * device. We are inserting a pause here so that userspace has time
     * to detect, initialize the new device, and can start listening to
     * the event, otherwise it will not notice the event we are about
     * to send. This pause is only needed in our example code!
     */
    sleep(1);

    device_ref_count++;

    debug_printf("Created touch input device with x-max=%d, y-max=%d, slots=%d\n", x_max, y_max, slots);

    /* Save sys name */
    do_ioctl(touch_fd, UI_GET_SYSNAME(50), sys_name);

    return 0;
}

void touch_destroy(void)
{
    /*
     * Give userspace some time to read the events before we destroy the
     * device with UI_DEV_DESTROY.
     */
    sleep(1);

    if (touch_fd < 0)
    {
        return;
    }

    debug_printf("Destroying touch input device\n");

    do_ioctl(touch_fd, UI_DEV_DESTROY);
    close(touch_fd);

    touch_fd = -1;

    sys_name[0] = 0;

    device_ref_count--;
}

const char* touch_sys_name(void)
{
    return sys_name;
}

void do_touch_tap(void *message)
{
    message_header_t *header = message;
    touch_tap_data_t *tap = message + sizeof(message_header_t);

    if (header->payload_length != sizeof(touch_tap_data_t))
    {
        warning_printf("Invalid payload length\n");
        return;
    }

    debug_printf("Touch tap at %d,%d for %d ms\n", tap->x, tap->y, tap->duration);
    touch_tap(tap->x, tap->y, tap->duration);

    msg_send_rsp_ok();
}

void do_touch_tap_request(uint32_t x, uint32_t y, uint32_t duration)
{
    void *message = NULL;
    touch_tap_data_t touch_tap_data;

    debug_printf("Sending tap message!\n");

    touch_tap_data.x = x;
    touch_tap_data.y = y;
    touch_tap_data.duration = duration;

    msg_create(&message, REQ_TOUCH_TAP, &touch_tap_data, sizeof(touch_tap_data_t));
    msg_send(message);
    msg_destroy(message);

    msg_receive_rsp_ok();
}

void do_touch_start(void *message)
{
    message_header_t *header = message;
    touch_start_data_t *data = message + sizeof(message_header_t);

    if (header->payload_length != sizeof(touch_start_data_t))
    {
        warning_printf("Invalid payload length\n");
        return;
    }

    touch_create(data->x_max, data->y_max, data->slots);

    msg_send_rsp_ok();
}

void do_touch_start_request(uint32_t x_max, uint32_t y_max, uint8_t slots)
{
    void *message = NULL;
    touch_start_data_t data;

    data.x_max = x_max;
    data.y_max = y_max;
    data.slots = slots;

    msg_create(&message, REQ_TOUCH_START, &data, sizeof(data));
    msg_send(message);
    msg_destroy(message);

    msg_receive_rsp_ok();
}

