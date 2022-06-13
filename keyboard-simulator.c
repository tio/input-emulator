/*
 * A simple keyboard simulator
 *
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <linux/uinput.h>
#include "script.h"

#define SHIFT (1 << 14)

/* Map list of supportd ASCII values (incomplete) */
static int16_t ascii_to_key_map[][2] =
{
    { 'a', KEY_A },
    { 'b', KEY_B },
    { 'c', KEY_C },
    { 'd', KEY_D },
    { 'e', KEY_E },
    { 'f', KEY_F },
    { 'g', KEY_G },
    { 'h', KEY_H },
    { 'i', KEY_I },
    { 'j', KEY_J },
    { 'k', KEY_K },
    { 'l', KEY_L },
    { 'm', KEY_M },
    { 'n', KEY_N },
    { 'o', KEY_O },
    { 'p', KEY_P },
    { 'q', KEY_Q },
    { 'r', KEY_R },
    { 's', KEY_S },
    { 't', KEY_T },
    { 'u', KEY_U },
    { 'v', KEY_V },
    { 'w', KEY_W },
    { 'x', KEY_X },
    { 'y', KEY_Y },
    { 'z', KEY_Z },

    { 'A', KEY_A | SHIFT },
    { 'B', KEY_B | SHIFT },
    { 'C', KEY_C | SHIFT },
    { 'D', KEY_D | SHIFT },
    { 'E', KEY_E | SHIFT },
    { 'F', KEY_F | SHIFT },
    { 'G', KEY_G | SHIFT },
    { 'H', KEY_H | SHIFT },
    { 'I', KEY_I | SHIFT },
    { 'J', KEY_J | SHIFT },
    { 'K', KEY_K | SHIFT },
    { 'L', KEY_L | SHIFT },
    { 'M', KEY_M | SHIFT },
    { 'N', KEY_N | SHIFT },
    { 'O', KEY_O | SHIFT },
    { 'P', KEY_P | SHIFT },
    { 'Q', KEY_Q | SHIFT },
    { 'R', KEY_R | SHIFT },
    { 'S', KEY_S | SHIFT },
    { 'T', KEY_T | SHIFT },
    { 'U', KEY_U | SHIFT },
    { 'V', KEY_V | SHIFT },
    { 'W', KEY_W | SHIFT },
    { 'X', KEY_X | SHIFT },
    { 'Y', KEY_Y | SHIFT },
    { 'Z', KEY_Z | SHIFT },

    { '0', KEY_0 },
    { '1', KEY_1 },
    { '2', KEY_2 },
    { '3', KEY_3 },
    { '4', KEY_4 },
    { '5', KEY_5 },
    { '6', KEY_6 },
    { '7', KEY_7 },
    { '8', KEY_8 },
    { '9', KEY_9 },

    { '!', KEY_1 | SHIFT },
    { '"', KEY_2 | SHIFT },
    { '#', KEY_3 | SHIFT },
    { '%', KEY_5 | SHIFT },
    { '&', KEY_6 | SHIFT },
    { '/', KEY_7 | SHIFT },
    { '(', KEY_8 | SHIFT },
    { ')', KEY_9 | SHIFT },
    { '=', KEY_0 | SHIFT },
    { '?', KEY_MINUS | SHIFT },

    { ' ', KEY_SPACE },
    { '-', KEY_SLASH },
    { '.', KEY_DOT },
    { ',', KEY_COMMA },
    
    { '\'', KEY_BACKSLASH },
    { '*', KEY_BACKSLASH | SHIFT },

    { '_', KEY_SLASH | SHIFT},
    { ':', KEY_DOT | SHIFT },
    { ';', KEY_COMMA | SHIFT },

    { '\r', KEY_ENTER },
    { '\t', KEY_TAB },
    { 0, 0 }, 
};

/* All input key codes known to man */
static const int key_list[] =
{
    KEY_ESC, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
    KEY_0, KEY_MINUS, KEY_EQUAL, KEY_BACKSPACE, KEY_TAB, KEY_Q, KEY_W, KEY_E,
    KEY_R, KEY_T, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P, KEY_LEFTBRACE,
    KEY_RIGHTBRACE, KEY_ENTER, KEY_LEFTCTRL, KEY_A, KEY_S, KEY_D, KEY_F, KEY_G,
    KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLON, KEY_APOSTROPHE, KEY_GRAVE,
    KEY_LEFTSHIFT, KEY_BACKSLASH, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N,
    KEY_M, KEY_COMMA, KEY_DOT, KEY_SLASH, KEY_RIGHTSHIFT, KEY_KPASTERISK,
    KEY_LEFTALT, KEY_SPACE, KEY_CAPSLOCK, KEY_F1, KEY_F2, KEY_F3, KEY_F4,
    KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_NUMLOCK,
    KEY_SCROLLLOCK, KEY_KP7, KEY_KP8, KEY_KP9, KEY_KPMINUS, KEY_KP4, KEY_KP5,
    KEY_KP6, KEY_KPPLUS, KEY_KP1, KEY_KP2, KEY_KP3, KEY_KP0, KEY_KPDOT,
    KEY_ZENKAKUHANKAKU, KEY_102ND, KEY_F11, KEY_F12, KEY_RO, KEY_KATAKANA,
    KEY_HIRAGANA, KEY_HENKAN, KEY_KATAKANAHIRAGANA, KEY_MUHENKAN, KEY_KPJPCOMMA,
    KEY_KPENTER, KEY_RIGHTCTRL, KEY_KPSLASH, KEY_SYSRQ, KEY_RIGHTALT,
    KEY_LINEFEED, KEY_HOME, KEY_UP, KEY_PAGEUP, KEY_LEFT, KEY_RIGHT, KEY_END,
    KEY_DOWN, KEY_PAGEDOWN, KEY_INSERT, KEY_DELETE, KEY_MACRO, KEY_MUTE,
    KEY_VOLUMEDOWN, KEY_VOLUMEUP, KEY_POWER, KEY_KPEQUAL, KEY_KPPLUSMINUS,
    KEY_PAUSE, KEY_SCALE, KEY_KPCOMMA, KEY_HANGEUL, KEY_HANGUEL, KEY_HANJA,
    KEY_YEN, KEY_LEFTMETA, KEY_RIGHTMETA, KEY_COMPOSE, KEY_STOP, KEY_AGAIN,
    KEY_PROPS, KEY_UNDO, KEY_FRONT, KEY_COPY, KEY_OPEN, KEY_PASTE, KEY_FIND,
    KEY_CUT, KEY_HELP, KEY_MENU, KEY_CALC, KEY_SETUP, KEY_SLEEP, KEY_WAKEUP,
    KEY_FILE, KEY_SENDFILE, KEY_DELETEFILE, KEY_XFER, KEY_PROG1, KEY_PROG2,
    KEY_WWW, KEY_MSDOS, KEY_COFFEE, KEY_SCREENLOCK, KEY_ROTATE_DISPLAY,
    KEY_DIRECTION, KEY_CYCLEWINDOWS, KEY_MAIL, KEY_BOOKMARKS, KEY_COMPUTER,
    KEY_BACK, KEY_FORWARD, KEY_CLOSECD, KEY_EJECTCD, KEY_EJECTCLOSECD,
    KEY_NEXTSONG, KEY_PLAYPAUSE, KEY_PREVIOUSSONG, KEY_STOPCD, KEY_RECORD,
    KEY_REWIND, KEY_PHONE, KEY_ISO, KEY_CONFIG, KEY_HOMEPAGE, KEY_REFRESH,
    KEY_EXIT, KEY_MOVE, KEY_EDIT, KEY_SCROLLUP, KEY_SCROLLDOWN, KEY_KPLEFTPAREN,
    KEY_KPRIGHTPAREN, KEY_NEW, KEY_REDO, KEY_F13, KEY_F14, KEY_F15, KEY_F16,
    KEY_F17, KEY_F18, KEY_F19, KEY_F20, KEY_F21, KEY_F22, KEY_F23, KEY_F24,
    KEY_PLAYCD, KEY_PAUSECD, KEY_PROG3, KEY_PROG4, KEY_DASHBOARD, KEY_SUSPEND,
    KEY_CLOSE, KEY_PLAY, KEY_FASTFORWARD, KEY_BASSBOOST, KEY_PRINT, KEY_HP,
    KEY_CAMERA, KEY_SOUND, KEY_QUESTION, KEY_EMAIL, KEY_CHAT, KEY_SEARCH,
    KEY_CONNECT, KEY_FINANCE, KEY_SPORT, KEY_SHOP, KEY_ALTERASE, KEY_CANCEL,
    KEY_BRIGHTNESSDOWN, KEY_BRIGHTNESSUP, KEY_MEDIA, KEY_SWITCHVIDEOMODE,
    KEY_KBDILLUMTOGGLE, KEY_KBDILLUMDOWN, KEY_KBDILLUMUP, KEY_SEND, KEY_REPLY,
    KEY_FORWARDMAIL, KEY_SAVE, KEY_DOCUMENTS, KEY_BATTERY, KEY_BLUETOOTH,
    KEY_WLAN, KEY_UWB, KEY_UNKNOWN, KEY_VIDEO_NEXT, KEY_VIDEO_PREV,
    KEY_BRIGHTNESS_CYCLE, KEY_BRIGHTNESS_AUTO, KEY_BRIGHTNESS_ZERO,
    KEY_DISPLAY_OFF, KEY_WWAN, KEY_WIMAX, KEY_RFKILL, KEY_MICMUTE, KEY_OK,
    KEY_SELECT, KEY_GOTO, KEY_CLEAR, KEY_POWER2, KEY_OPTION, KEY_INFO, KEY_TIME,
    KEY_VENDOR, KEY_ARCHIVE, KEY_PROGRAM, KEY_CHANNEL, KEY_FAVORITES, KEY_EPG,
    KEY_PVR, KEY_MHP, KEY_LANGUAGE, KEY_TITLE, KEY_SUBTITLE, KEY_ANGLE,
    KEY_ZOOM, KEY_MODE, KEY_KEYBOARD, KEY_SCREEN, KEY_PC, KEY_TV, KEY_TV2,
    KEY_VCR, KEY_VCR2, KEY_SAT, KEY_SAT2, KEY_CD, KEY_TAPE, KEY_RADIO,
    KEY_TUNER, KEY_PLAYER, KEY_TEXT, KEY_DVD, KEY_AUX, KEY_MP3, KEY_AUDIO,
    KEY_VIDEO, KEY_DIRECTORY, KEY_LIST, KEY_MEMO, KEY_CALENDAR, KEY_RED,
    KEY_GREEN, KEY_YELLOW, KEY_BLUE, KEY_CHANNELUP, KEY_CHANNELDOWN, KEY_FIRST,
    KEY_LAST, KEY_AB, KEY_NEXT, KEY_RESTART, KEY_SLOW, KEY_SHUFFLE, KEY_BREAK,
    KEY_PREVIOUS, KEY_DIGITS, KEY_TEEN, KEY_TWEN, KEY_VIDEOPHONE, KEY_GAMES,
    KEY_ZOOMIN, KEY_ZOOMOUT, KEY_ZOOMRESET, KEY_WORDPROCESSOR, KEY_EDITOR,
    KEY_SPREADSHEET, KEY_GRAPHICSEDITOR, KEY_PRESENTATION, KEY_DATABASE,
    KEY_NEWS, KEY_VOICEMAIL, KEY_ADDRESSBOOK, KEY_MESSENGER, KEY_DISPLAYTOGGLE,
    KEY_BRIGHTNESS_TOGGLE, KEY_SPELLCHECK, KEY_LOGOFF, KEY_DOLLAR, KEY_EURO,
    KEY_FRAMEBACK, KEY_FRAMEFORWARD, KEY_CONTEXT_MENU, KEY_MEDIA_REPEAT,
    KEY_10CHANNELSUP, KEY_10CHANNELSDOWN, KEY_IMAGES, KEY_DEL_EOL, KEY_DEL_EOS,
    KEY_INS_LINE, KEY_DEL_LINE, KEY_FN, KEY_FN_ESC, KEY_FN_F1, KEY_FN_F2,
    KEY_FN_F3, KEY_FN_F4, KEY_FN_F5, KEY_FN_F6, KEY_FN_F7, KEY_FN_F8, KEY_FN_F9,
    KEY_FN_F10, KEY_FN_F11, KEY_FN_F12, KEY_FN_1, KEY_FN_2, KEY_FN_D, KEY_FN_E,
    KEY_FN_F, KEY_FN_S, KEY_FN_B, KEY_BRL_DOT1, KEY_BRL_DOT2, KEY_BRL_DOT3,
    KEY_BRL_DOT4, KEY_BRL_DOT5, KEY_BRL_DOT6, KEY_BRL_DOT7, KEY_BRL_DOT8,
    KEY_BRL_DOT9, KEY_BRL_DOT10, KEY_NUMERIC_0, KEY_NUMERIC_1, KEY_NUMERIC_2,
    KEY_NUMERIC_3, KEY_NUMERIC_4, KEY_NUMERIC_5, KEY_NUMERIC_6, KEY_NUMERIC_7,
    KEY_NUMERIC_8, KEY_NUMERIC_9, KEY_NUMERIC_STAR, KEY_NUMERIC_POUND,
    KEY_NUMERIC_A, KEY_NUMERIC_B, KEY_NUMERIC_C, KEY_NUMERIC_D,
    KEY_CAMERA_FOCUS, KEY_WPS_BUTTON, KEY_TOUCHPAD_TOGGLE, KEY_TOUCHPAD_ON,
    KEY_TOUCHPAD_OFF, KEY_CAMERA_ZOOMIN, KEY_CAMERA_ZOOMOUT, KEY_CAMERA_UP,
    KEY_CAMERA_DOWN, KEY_CAMERA_LEFT, KEY_CAMERA_RIGHT, KEY_ATTENDANT_ON,
    KEY_ATTENDANT_OFF, KEY_ATTENDANT_TOGGLE, KEY_LIGHTS_TOGGLE, KEY_ALS_TOGGLE,
    KEY_BUTTONCONFIG, KEY_TASKMANAGER, KEY_JOURNAL, KEY_CONTROLPANEL,
    KEY_APPSELECT, KEY_SCREENSAVER, KEY_VOICECOMMAND, KEY_BRIGHTNESS_MIN,
    KEY_BRIGHTNESS_MAX, KEY_KBDINPUTASSIST_PREV, KEY_KBDINPUTASSIST_NEXT,
    KEY_KBDINPUTASSIST_PREVGROUP, KEY_KBDINPUTASSIST_NEXTGROUP,
    KEY_KBDINPUTASSIST_ACCEPT, KEY_KBDINPUTASSIST_CANCEL, KEY_RIGHT_UP,
    KEY_RIGHT_DOWN, KEY_LEFT_UP, KEY_LEFT_DOWN, KEY_ROOT_MENU,
    KEY_MEDIA_TOP_MENU, KEY_NUMERIC_11, KEY_NUMERIC_12, KEY_AUDIO_DESC,
    KEY_3D_MODE, KEY_NEXT_FAVORITE, KEY_STOP_RECORD, KEY_PAUSE_RECORD, KEY_VOD,
    KEY_UNMUTE, KEY_FASTREVERSE, KEY_SLOWREVERSE, KEY_DATA, KEY_MIN_INTERESTING
};

void emit_event(int fd, uint16_t type, uint16_t code, int32_t value, bool syn_report)
{
    struct input_event ie =
    {
        .type = type,
        .code = code,
        .value = value,
    };

    write(fd, &ie, sizeof(ie));

    if (syn_report)
    {
        ie.type = EV_SYN;
        ie.code = SYN_REPORT;
        ie.value = 0;

        write(fd, &ie, sizeof(ie));
    }
}

void key_press(int fd, int key)
{
    emit_event(fd, EV_KEY, key, 1, true);
}

void key_release(int fd, int key)
{
    emit_event(fd, EV_KEY, key, 0, true);
}

int16_t ascii_to_key(uint8_t c)
{
    int i = 0;

    while (ascii_to_key_map[i][0] != 0)
    {
        if (ascii_to_key_map[i][0] == c)
        {
            return ascii_to_key_map[i][1];
        }
        i++;
    }

    return -1;
}

void type_key(int fd, int16_t key)
{
    bool shift = false;

    if (key < 0)
    {
        fprintf(stderr, "Invalid key\n");
        return;
    }

    // Apply shift if required
    if (SHIFT & key)
    {
        shift = true;

        // Remove shift bit
        key &= ~(SHIFT);
    }

    if (shift)
    {
        key_press(fd, KEY_LEFTSHIFT);
    }

    key_press(fd, key);
    usleep(12*1000);
    key_release(fd, key);

    if (shift)
    {
        key_release(fd, KEY_LEFTSHIFT);
    }
}

void type_string(int fd, char *string, int delay)
{
    int i = 0;

    while (string[i] != 0)
    {
        type_key(fd, ascii_to_key(string[i]));
        usleep(delay * 1000);
        i++;
    }
}

int uinput_create(void)
{
    struct uinput_setup usetup;
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

    /* Configure device to pass the following keyboard events */
    ioctl(fd, UI_SET_EVBIT, EV_KEY);

    for (int i=0; i<sizeof(key_list)/sizeof(int); i++)
    {
        if (ioctl(fd, UI_SET_KEYBIT, key_list[i]))
        {
            fprintf(stderr, "UI_SET_KEYBIT %d failed\n", i);
        }
    }

    /* Configure device properties */
    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1234;
    usetup.id.product = 0x5678;
    usetup.id.version = 1;
    strcpy(usetup.name, "Keyboard simulator");

    /* Create device */
    ioctl(fd, UI_DEV_SETUP, &usetup);
    ioctl(fd, UI_DEV_CREATE);

    /* Wait for kernel to finish creating device */
    sleep(1);

    return fd;
}

void uinput_destroy(int fd)
{
    ioctl(fd, UI_DEV_DESTROY);
    close(fd);
}

int main(void)
{
    /* Create virtual input device */
    int fd = uinput_create();

    /* Run keyboard script */
    keyboard_script(fd);

    /* Wait for userspace to finish reading events */
    sleep(1);

    /* Destroy virtual input device */
    uinput_destroy(fd);

    return 0;
}
