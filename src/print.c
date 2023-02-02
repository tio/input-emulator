/*
 * Copyright (C) 2023  DEIF A/S
 */

#include <stdio.h>
#include "print.h"
#include "misc.h"

void debug_print_hex_dump(void *data, int length)
{

#ifdef DEBUG

    char *bufferp = data;
    int i;

    for (i=0; i<length; i++)
    {
        if (i%10 == 0)
        {
            if (i == 0)
            {
                debug_printf("");
            }
            else
            {
                debug_printf_raw("\n");
                debug_printf("");
            }
        }
        debug_printf_raw("0x%02x ", (unsigned char) bufferp[i]);
    }
    debug_printf_raw("\n");

#else

    UNUSED(data);
    UNUSED(length);

#endif

}
