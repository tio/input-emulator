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

#pragma once

#include <stdio.h>

#define error_printf(format, args...) \
    fprintf(stderr, "Error: " format, ## args)
#define error_printf_raw(format, args...) \
    fprintf(stderr, "" format, ## args)

#define warning_printf(format, args...) \
    fprintf(stderr, "Warning: " format, ## args)
#define warning_printf_raw(format, args...) \
    fprintf(stderr, "" format, ## args)

// #define DEBUG

#ifdef DEBUG
#define debug_printf(format, args...) \
    fprintf(stdout, "[debug] " format, ## args)
#define debug_printf_raw(format, args...) \
    fprintf(stdout, "" format, ## args)
#else
#define debug_printf(format, args...)
#define debug_printf_raw(format, args...)
#endif

void debug_print_hex_dump(void *data, int length);
