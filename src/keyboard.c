/*
 * Copyright (C) 2022  Martin Lund
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

#include <linux/input-event-codes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <linux/uinput.h>
#include <wchar.h>
#include "event.h"
#include "service.h"
#include "message.h"
#include "print.h"
#include "config.h"
#include "keyboard.h"
#include "misc.h"

#define SHIFT  (1 << 14)
#define ALT_GR (1 << 13)

static int keyboard_fd = -1;
static char sys_name[SYS_NAME_LENGTH_MAX];
static uint32_t kbd_type_delay;

/* Map list of supportd wchar values (incomplete) */
static struct wchar_to_key_map_t
{
    wchar_t wchar;
    uint32_t key;
    uint32_t modifier;
}
wchar_to_key_map_dk[] =
{
    { 32, KEY_SPACE, 0}, // ' '
    { 33, KEY_1, KEY_LEFTSHIFT}, // !
    { 34, KEY_2, KEY_LEFTSHIFT}, // "
    { 35, KEY_3, KEY_LEFTSHIFT}, // #
    { 36, KEY_4, KEY_LEFTSHIFT}, // $
    { 37, KEY_5, KEY_LEFTSHIFT}, // %
    { 38, KEY_6, KEY_LEFTSHIFT}, // &
    { 39, KEY_BACKSLASH, 0}, // '
    { 40, KEY_8, KEY_LEFTSHIFT}, // (
    { 41, KEY_9, KEY_LEFTSHIFT}, // )
    { 42, KEY_BACKSLASH, KEY_LEFTSHIFT}, // *
    { 43, KEY_MINUS, 0}, // +
    { 44, KEY_COMMA, 0}, // ,
    { 45, KEY_SLASH, 0}, // -
    { 46, KEY_DOT, 0}, // .
    { 47, KEY_7, KEY_LEFTSHIFT}, // /
    { 48, KEY_0, 0}, // 0
    { 49, KEY_1, 0}, // 1
    { 50, KEY_2, 0}, // 2
    { 51, KEY_3, 0}, // 3
    { 52, KEY_4, 0}, // 4
    { 53, KEY_5, 0}, // 5
    { 54, KEY_6, 0}, // 6
    { 55, KEY_7, 0}, // 7
    { 56, KEY_8, 0}, // 8
    { 57, KEY_9, 0}, // 9
    { 58, KEY_DOT, KEY_LEFTSHIFT}, // :
    { 59, KEY_SEMICOLON, 0}, // ;
    { 60, KEY_102ND, 0}, // <
    { 61, KEY_0, KEY_LEFTSHIFT}, // =
    { 62, KEY_102ND, KEY_LEFTSHIFT}, // >
    { 63, KEY_MINUS, KEY_LEFTSHIFT}, // ?
    { 64, KEY_2, KEY_LEFTALT}, // @
    { 65, KEY_A, KEY_LEFTSHIFT}, // A
    { 66, KEY_B, KEY_LEFTSHIFT}, // B
    { 67, KEY_C, KEY_LEFTSHIFT}, // C
    { 68, KEY_D, KEY_LEFTSHIFT}, // D
    { 69, KEY_E, KEY_LEFTSHIFT}, // E
    { 70, KEY_F, KEY_LEFTSHIFT}, // F
    { 71, KEY_G, KEY_LEFTSHIFT}, // G
    { 72, KEY_H, KEY_LEFTSHIFT}, // H
    { 73, KEY_I, KEY_LEFTSHIFT}, // I
    { 74, KEY_J, KEY_LEFTSHIFT}, // J
    { 75, KEY_K, KEY_LEFTSHIFT}, // K
    { 76, KEY_L, KEY_LEFTSHIFT}, // L
    { 77, KEY_M, KEY_LEFTSHIFT}, // M
    { 78, KEY_N, KEY_LEFTSHIFT}, // N
    { 79, KEY_O, KEY_LEFTSHIFT}, // O
    { 80, KEY_P, KEY_LEFTSHIFT}, // P
    { 81, KEY_Q, KEY_LEFTSHIFT}, // Q
    { 82, KEY_R, KEY_LEFTSHIFT}, // R
    { 83, KEY_S, KEY_LEFTSHIFT}, // S
    { 84, KEY_T, KEY_LEFTSHIFT}, // T
    { 85, KEY_U, KEY_LEFTSHIFT}, // U
    { 86, KEY_V, KEY_LEFTSHIFT}, // V
    { 87, KEY_W, KEY_LEFTSHIFT}, // W
    { 88, KEY_X, KEY_LEFTSHIFT}, // X
    { 89, KEY_Y, KEY_LEFTSHIFT}, // Y
    { 90, KEY_Z, KEY_LEFTSHIFT}, // Z
    { 91, KEY_8, KEY_RIGHTALT}, // [
    { 92, KEY_102ND, KEY_LEFTALT}, // '\'
    { 93, KEY_9, KEY_LEFTALT}, // ]
    { 94, KEY_RIGHTBRACE, KEY_LEFTSHIFT}, // ^
    { 95, KEY_SLASH, KEY_LEFTSHIFT}, // _
    { 96, KEY_EQUAL, KEY_LEFTSHIFT}, // `
    { 97, KEY_A, 0}, // a
    { 98, KEY_B, 0}, // b
    { 99, KEY_C, 0}, // c
    { 100, KEY_D, 0}, // d
    { 101, KEY_E, 0}, // e
    { 102, KEY_F, 0}, // f
    { 103, KEY_G, 0}, // g
    { 104, KEY_H, 0}, // h
    { 105, KEY_I, 0}, // i
    { 106, KEY_J, 0}, // j
    { 107, KEY_K, 0}, // k
    { 108, KEY_L, 0}, // l
    { 109, KEY_M, 0}, // m
    { 110, KEY_N, 0}, // n
    { 111, KEY_O, 0}, // o
    { 112, KEY_P, 0}, // p
    { 113, KEY_Q, 0}, // q
    { 114, KEY_R, 0}, // r
    { 115, KEY_S, 0}, // s
    { 116, KEY_T, 0}, // t
    { 117, KEY_U, 0}, // u
    { 118, KEY_V, 0}, // v
    { 119, KEY_W, 0}, // w
    { 120, KEY_X, 0}, // x
    { 121, KEY_Y, 0}, // y
    { 122, KEY_Z, 0}, // z
    { 123, KEY_7, KEY_RIGHTALT}, // {
    { 124, KEY_EQUAL, KEY_RIGHTALT}, // |
    { 125, KEY_7, KEY_RIGHTALT}, // }
    { 126, KEY_RIGHTBRACE, KEY_RIGHTALT}, // ~

    { 180, KEY_EQUAL, 0}, // `

    { 197, KEY_LEFTBRACE, KEY_LEFTSHIFT}, // Å
    { 198, KEY_SEMICOLON, KEY_LEFTSHIFT}, // Æ
    { 216, KEY_APOSTROPHE, KEY_LEFTSHIFT}, // Ø
    { 229, KEY_LEFTBRACE, 0}, // å
    { 230, KEY_SEMICOLON, 0}, // æ
    { 248, KEY_APOSTROPHE, 0}, // ø

    { 0, 0, 0}, // End of list
};

static struct alias_to_key_map_t
{
    wchar_t *alias;
    uint32_t key;
    uint32_t modifier;
}
alias_to_key_map_dk[] =
{
    { L"alt",       KEY_LEFTALT,   0},
    { L"altgr",     KEY_RIGHTALT,  0},
    { L"backspace", KEY_BACKSPACE, 0},
    { L"capslock",  KEY_CAPSLOCK,  0},
    { L"compose",   KEY_COMPOSE,   0},
    { L"ctrl",      KEY_LEFTCTRL,  0},
    { L"delete",    KEY_DELETE,    0},
    { L"down",      KEY_DOWN,      0},
    { L"end",       KEY_END,       0},
    { L"enter",     KEY_ENTER,     0},
    { L"esc",       KEY_ESC,       0},
    { L"f1",        KEY_F1,        0},
    { L"f2",        KEY_F2,        0},
    { L"f3",        KEY_F3,        0},
    { L"f4",        KEY_F4,        0},
    { L"f5",        KEY_F5,        0},
    { L"f6",        KEY_F6,        0},
    { L"f7",        KEY_F7,        0},
    { L"f8",        KEY_F8,        0},
    { L"f9",        KEY_F9,        0},
    { L"f10",       KEY_F10,       0},
    { L"f11",       KEY_F11,       0},
    { L"f12",       KEY_F12,       0},
    { L"f13",       KEY_F13,       0},
    { L"f14",       KEY_F14,       0},
    { L"f15",       KEY_F15,       0},
    { L"f16",       KEY_F16,       0},
    { L"f17",       KEY_F17,       0},
    { L"f18",       KEY_F18,       0},
    { L"f19",       KEY_F19,       0},
    { L"f20",       KEY_F20,       0},
    { L"help",      KEY_HELP,      0},
    { L"home",      KEY_HOME,      0},
    { L"left",      KEY_LEFT,      0},
    { L"meta",      KEY_LEFTMETA,  0},
    { L"playpause", KEY_PLAYPAUSE, 0},
    { L"pgdn",      KEY_PAGEDOWN,  0},
    { L"pgup",      KEY_PAGEUP,    0},
    { L"right",     KEY_RIGHT,     0},
    { L"shift",     KEY_LEFTSHIFT, 0},
    { L"space",     KEY_SPACE,     0},
    { L"stopcd",    KEY_STOPCD,    0},
    { L"tab",       KEY_TAB,       0},
    { L"up",        KEY_UP,        0},

    { NULL, 0, 0}, // End of list
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


void keyboard_press(uint32_t key)
{
    /* Do nothing if no device */
    if (keyboard_fd < 0)
    {
        return;
    }

    debug_printf("Press key %d\n", key);

    emit(keyboard_fd, EV_KEY, key, 1);
    emit(keyboard_fd, EV_SYN, SYN_REPORT, 0);
}

void keyboard_release(uint32_t key)
{
    /* Do nothing if no device */
    if (keyboard_fd < 0)
    {
        return;
    }

    debug_printf("Release key %d\n", key);

    emit(keyboard_fd, EV_KEY, key, 0);
    emit(keyboard_fd, EV_SYN, SYN_REPORT, 0);
}

int wchar_to_key(wchar_t wc, uint32_t *key, uint32_t *modifier)
{
    int i = 0;

    /* Hardcoded to DK mapping for now */
    while (wchar_to_key_map_dk[i].wchar != 0)
    {
        if (wchar_to_key_map_dk[i].wchar == wc)
        {
            // Found key
            *key = wchar_to_key_map_dk[i].key;
            *modifier = wchar_to_key_map_dk[i].modifier;
            return 0;
        }
        i++;
    }

    return -1;
}

int alias_to_key(wchar_t *wcs, uint32_t *key, uint32_t *modifier)
{
    int i = 0;

    /* Hardcoded to DK mapping for now */
    while (alias_to_key_map_dk[i].alias != NULL)
    {
        if (wcscmp(wcs, alias_to_key_map_dk[i].alias) == 0)
        {
            // Found key
            *key = alias_to_key_map_dk[i].key;
            *modifier = alias_to_key_map_dk[i].modifier;
            return 0;
        }
        i++;
    }

    return -1;
}

void wchar_or_alias_to_key(wchar_t *wcs, uint32_t *key)
{
    uint32_t modifier;

    if (wcslen(wcs) == 1)
    {
        wchar_to_key(wcs[0], key, &modifier);
    }
    else if (alias_to_key(wcs, key, &modifier) == 0)
    {
    }
    else
    {
        error_printf("Invalid input\n");
        exit(EXIT_FAILURE);
    }
}

int keyboard_create(uint32_t type_delay)
{
    struct uinput_setup usetup;

    if (keyboard_fd >= 0)
    {
        /* Keyboard already started */
        return -1;
    }

    kbd_type_delay = type_delay;

    keyboard_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (keyboard_fd < 0)
    {
        error_printf("Could not open /dev/uinput (%s)\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Configure device to pass the following keyboard events */
    do_ioctl(keyboard_fd, UI_SET_EVBIT, EV_KEY);

    for (unsigned long i=0; i<(sizeof(key_list)/sizeof(int)); i++)
    {
        if (ioctl(keyboard_fd, UI_SET_KEYBIT, key_list[i]))
        {
            error_printf("UI_SET_KEYBIT %ld failed\n", i);
        }
    }

    /* Configure device properties */
    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1234;
    usetup.id.product = 0x5678;
    usetup.id.version = 1;
    strcpy(usetup.name, "Keyboard emulator");

    /* Create device */
    do_ioctl(keyboard_fd, UI_DEV_SETUP, &usetup);
    do_ioctl(keyboard_fd, UI_DEV_CREATE);

    /* Wait for kernel to finish creating device */
    sleep(1);

    device_ref_count++;

    debug_printf("Created keyboard input device\n");

    /* Save sys name */
    do_ioctl(keyboard_fd, UI_GET_SYSNAME(50), sys_name);

    return 0;
}

bool keyboard_online(void)
{
    if (keyboard_fd >= 0)
    {
        return true;
    }

    return false;
}

void keyboard_destroy(void)
{
    /*
     * Give userspace some time to read the events before we destroy the
     * device with UI_DEV_DESTROY.
     */
    sleep(1);

    if (keyboard_fd < 0)
    {
        return;
    }
        debug_printf("Destroying keyboard input device\n");

        do_ioctl(keyboard_fd, UI_DEV_DESTROY);
        close(keyboard_fd);

        keyboard_fd = -1;

        sys_name[0] = 0;

        device_ref_count--;
}

const char* keyboard_sys_name(void)
{
    return sys_name;
}

uint32_t keyboard_type_delay(void)
{
    return kbd_type_delay;
}

void do_keyboard_start(void *message)
{
    message_header_t *header = message;
    keyboard_start_data_t *data = message + sizeof(message_header_t);

    if (header->payload_length != sizeof(keyboard_start_data_t))
    {
        warning_printf("Warning: Invalid payload length\n");
        return;
    }

    keyboard_create(data->type_delay);

    msg_send_rsp_ok();
}

void do_keyboard_start_request(uint32_t type_delay)
{
    void *message = NULL;
    keyboard_start_data_t data;

    debug_printf("Sending keyboard start message!\n");

    data.type_delay = type_delay;

    msg_create(&message, REQ_KBD_START, &data, sizeof(data));
    msg_send(message);
    msg_destroy(message);

    msg_receive_rsp_ok();
}

void do_keyboard_keydown(void *message)
{
    message_header_t *header = message;
    uint32_t *key = message + sizeof(message_header_t);

    if (header->payload_length != sizeof(uint32_t))
    {
        warning_printf("Warning: Invalid payload length-\n");
        return;
    }

    keyboard_press(*key);

    msg_send_rsp_ok();
}

void do_keyboard_keydown_request(uint32_t key)
{
    void *message = NULL;

    msg_create(&message, REQ_KBD_KEYDOWN, &key, sizeof(key));
    msg_send(message);
    msg_destroy(message);

    msg_receive_rsp_ok();
}

void do_keyboard_keyup(void *message)
{
    message_header_t *header = message;
    uint32_t *key = message + sizeof(message_header_t);

    if (header->payload_length != sizeof(uint32_t))
    {
        warning_printf("Warning: Invalid payload length-\n");
        return;
    }

    keyboard_release(*key);

    msg_send_rsp_ok();
}

void do_keyboard_keyup_request(uint32_t key)
{
    void *message = NULL;

    msg_create(&message, REQ_KBD_KEYUP, &key, sizeof(key));
    msg_send(message);
    msg_destroy(message);

    msg_receive_rsp_ok();
}

void do_keyboard_key(void *message)
{
    message_header_t *header = message;
    uint32_t *key = message + sizeof(message_header_t);

    if (header->payload_length != sizeof(uint32_t))
    {
        warning_printf("Warning: Invalid payload length-\n");
        return;
    }

    keyboard_press(*key);
    usleep(kbd_type_delay*1000);
    keyboard_release(*key);

    msg_send_rsp_ok();
}

void do_keyboard_key_request(uint32_t key)
{
    void *message = NULL;

    msg_create(&message, REQ_KBD_KEY, &key, sizeof(key));
    msg_send(message);
    msg_destroy(message);

    msg_receive_rsp_ok();
}

void do_keyboard_type(void *message)
{
    const wchar_t *wc_string = message + sizeof(message_header_t);
    size_t length = wcslen(wc_string) + 1;
    uint32_t modifier;
    uint32_t key;

    // Dump data received
    message_header_t *header = message;
    debug_printf("Dumping received payload:\n");
    debug_print_hex_dump((void *)wc_string, header->payload_length);

    /* Translate each wide character in wc string to uinput key stroke with any
     * modifiers (ALT_LEFTSHIFT, ALT_GR, etc) required */

    for (size_t i = 0; i<(length); i++)
    {
        debug_printf("wchar: 0x%x\n", wc_string[i]);
        if (wchar_to_key(wc_string[i], &key, &modifier) == 0)
        {
            debug_printf("wchar: %d, key: %d, modifier: %d\n", wc_string[i], key, modifier);
            if (modifier)
            {
                keyboard_press(modifier);
            }

            keyboard_press(key);
            usleep(kbd_type_delay*1000);
            keyboard_release(key);

            if (modifier)
            {
                keyboard_release(modifier);
            }
        }
    }

    msg_send_rsp_ok();
}

void do_keyboard_type_request(const wchar_t *wc_string)
{
    void *message = NULL;
    uint32_t wc_string_byte_length = sizeof(wchar_t) * (wcslen(wc_string) + 1);

    // Dump data sent
    debug_printf("Dumping send payload:\n");
    debug_print_hex_dump((void *)wc_string, wc_string_byte_length);

    msg_create(&message, REQ_KBD_TYPE, (void *) wc_string, wc_string_byte_length);
    msg_send(message);
    msg_destroy(message);

    msg_receive_rsp_ok();
}
