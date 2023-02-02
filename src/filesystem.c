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

#include <stdbool.h>
#include <sys/stat.h>

bool file_socket_exists(const char *filename)
{
   struct stat st;

   if (stat(filename, &st) != 0)
   {
      return false;
   }
   else if (!S_ISSOCK(st.st_mode))
   {
      return false;
   }

   return true;
}
