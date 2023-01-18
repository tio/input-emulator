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
#include <wchar.h>

wchar_t *convert_mbs_to_wcs(const char *string)
{
    wchar_t *wcs;
    size_t mbs_length;

    mbs_length = mbstowcs(NULL, string, 0);
    if (mbs_length == (size_t) -1)
    {
        perror("mbstowcs");
        exit(EXIT_FAILURE);
    }

    wcs = calloc(mbs_length + 1, sizeof(wchar_t));
    if (wcs == NULL)
    {
        perror("calloc");
        exit(EXIT_FAILURE);
    }

    if (mbstowcs(wcs, string, mbs_length + 1) == (size_t) -1)
    {
        perror("mbstowcs");
        exit(EXIT_FAILURE);
    }

    return wcs;
}
