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
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include "filesystem.h"
#include "message.h"
#include "options.h"
#include "keyboard.h"
#include "mouse.h"
#include "touch.h"
#include "print.h"
#include "misc.h"

int device_ref_count = 0;

bool devices_online(void)
{
    if (device_ref_count > 0)
    {
        return true;
    }

    return false;
}

bool service_running(void)
{
   return file_exists("/dev/mqueue/input-emulator");
}

void daemonize(void)
{
    pid_t pid, sid;

    /* Already a daemon */
    if ( getppid() == 1 ) return;

    /* Fork off the parent process */
    pid = fork();
    if (pid < 0)
    {
        exit(EXIT_FAILURE);
    }

    /* If we got a good PID, then we can exit the parent process. */
    if (pid > 0)
    {
        exit(EXIT_SUCCESS);
    }

    /* At this point we are executing as the child process */

    /* Change the file mode mask */
    umask(0);

    /* Create a new SID for the child process (child becomes session leader)*/
    sid = setsid();
    if (sid < 0)
    {
        error_printf("Failed to become session leader\n");
        exit(EXIT_FAILURE);
    }

    /* Change the current working directory.  This prevents the current
       directory from being locked; hence not being able to remove it. */
    if ((chdir("/")) < 0)
    {
        error_printf("Failed to change directory\n");
        exit(EXIT_FAILURE);
    }


    /* Close all open file descriptors */
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>=0; x--)
    {
        debug_printf("Closing file descriptor %d\n", x);
        close (x);
    }

    /* Redirect standard files to /dev/null */
    freopen( "/dev/null", "r", stdin);
    freopen( "/dev/null", "w", stdout);
    freopen( "/dev/null", "w", stderr);
}

void do_service_stop_request(device_t device)
{
    void *message = NULL;

    debug_printf("Sending stop message!\n");

    msg_create(&message, REQ_STOP, &device, sizeof(device));

    msg_send(message);
    msg_destroy(message);
}

void do_service_stop(void *message)
{
    message_header_t *header = message;
    device_t *device = message + sizeof(message_header_t);

    if (header->payload_length != sizeof(device_t))
    {
        warning_printf("Invalid payload length\n");
        return;
    }

    switch(*device)
    {
        case DEV_KEYBOARD:
            keyboard_destroy();
            break;

        case DEV_MOUSE:
            mouse_destroy();
            break;

        case DEV_TOUCH:
            touch_destroy();
            break;

        case DEV_NONE:
            break;
    }
}

void do_service_status(void *message)
{
    char rsp_text[400];
    char sys_path[] = "/sys/devices/virtual/input";
    char *rsp_text_p = rsp_text;

    sprintf(rsp_text_p, "Online devices:\n");
    rsp_text_p += strlen(rsp_text_p);

    if (keyboard_online())
    {
        sprintf(rsp_text_p, "  kbd: %s/%s\n", sys_path, keyboard_sys_name());
        rsp_text_p += strlen(rsp_text_p);
    }

    if (mouse_online())
    {
        sprintf(rsp_text_p, "mouse: %s/%s\n", sys_path, mouse_sys_name());
        rsp_text_p += strlen(rsp_text_p);
    }

    if (touch_online())
    {
        sprintf(rsp_text_p, "touch: %s/%s\n", sys_path, touch_sys_name());
    }

    // Send response
    msg_create(&message, RSP_STATUS, rsp_text, strlen(rsp_text));
    msg_send(message);
    msg_destroy(message);
}

void do_service_status_request(void)
{
    void *message = NULL;
    message_header_t *header;

    msg_create(&message, REQ_STATUS, NULL, 0);
    msg_send(message);
    msg_destroy(message);

    // Receive response
    msg_receive(&message);
    header = message;
    if (header->type != RSP_STATUS)
    {
        warning_printf("Invalid message type received\n");
    }

    char *rsp_text = message + sizeof(message_header_t);

    printf("%s", rsp_text);
    msg_destroy(message);
}
