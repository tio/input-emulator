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

#include <stdlib.h>
#include <signal.h>
#include "print.h"

void signal_handler(int signum)
{
    switch (signum)
    {
        case SIGHUP:
            debug_printf("Received SIGHUP signal!\n");
            break;
        case SIGINT:
            debug_printf("Received SIGINT signal!\n");
            break;
    }
    exit(EXIT_FAILURE);
}

void signal_handlers_install(void)
{
    signal(SIGHUP, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGPIPE, SIG_IGN);
}
