# input-emulator - A scriptable input emulator

[![](https://img.shields.io/github/v/release/tio/input-emulator?sort=semver)](https://github.com/tio/input-emulator/releases)
[![](https://img.shields.io/tokei/lines/github/tio/input-emulator)](https://github.com/tio/input-emulator)

## 1. Introduction

A scriptable input emulator for Linux which instructs the kernel to create
virtual keyboard, mouse, and touch input devices through which one can perform
various high level actions (typing, movement, gestures, etc.) via command-line
or script.

Similar actions can be performed with existing tools but they are either
bound to X (eg. xdotool) or operates with low level events (evemu-create,
evemu-event, etc.) which make scripted automation not as easy.

### 1.1 Motivation

To make a simple input emulator for test automation.

input-emulator was originally named keyboard-simulator and started as part of
the tio project to script keyboard input to automate generation of tio
demonstration gifs.

## 2. features

 * Emulates the 3 arch type input devices: keyboard, mouse, and touch
 * Perform device actions via command-line
    * Keyboard actions: type, key, keydown, keyup
    * Mouse actions: move, click, down, up
    * Touch actions: tap
 * Start/stop individual input device
 * Input devices are maintained by background service (default)
    * Allows stable input device name
    * Status of service can be queried via command-line
 * Documented via man page
 * Supports various keyboard layouts (TODO, limited to DK for now)
 * Shell completion support (TODO)
 * Perform actions via script (TODO)

## 3. Usage

### 3.1 Command-line

The command-line interface is straightforward as reflected in the output from 'input-emulator --help':

```
Usage: input-emulator [--version] [--help] <command> [<arguments>]

  -v, --version                      Display version
  -h, --help                         Display help

Available commands:
  start [<options>] kbd|mouse|touch  Create virtual input device
  kbd <action> [args]                Do keyboard action
  mouse <action> [args]              Do mouse action
  touch <action> [args]              Do touch action
  status                             Show status of virtual input devices
  stop kbd|mouse|touch|all           Destroy virtual input device

Start options:
  -x, --x-max <int>                  Maximum x-coordinate (only for mouse and touch)
  -y, --y-max <int>                  Maximum y-coordinate (only for mouse and touch)
  -s, --slots <int>                  Maximum number of slots (fingers) recognized (only for touch)
  -n, --no-daemonize                 Run in foreground

Keyboard actions:
  type '<string>'                    Type string
  key <key>                          Stroke key
  keydown <key>                      Press and hold key
  keyup <key>                        Release key

Mouse actions:
  move <x> <y>                       Move mouse x,y relative
  click left|middle|right            Click mouse button
  down left|middle|right             Push mouse button down
  up left|middle|right               Release mouse button

Touch actions:
  tap <x> <y>                        Tap at x,y coordinate
```

### 3.2 Examples

#### 3.2.1 Touch example
```
 $ input-emulator start touch --x-max 2560 --y-max 1440 --slots 4
 $ input-emulator touch tap 1280 720
 $ input-emulator stop touch
```
#### 3.2.2 Mouse example
```
 $ input-emulator start mouse --x-max 2560 --y-max 1440
 $ input-emulator mouse move 200 -300
 $ input-emulator mouse click left
 $ input-emulator mouse down right
 $ input-emulator mouse up right
 $ input-emulator mouse scroll -1
 $ input-emulator stop
```
#### 3.2.3 Keyboard example
```
 $ input-emulator start kbd
 $ input-emulator kbd type 'hello there'
 $ input-emulator kbd keydown ctrl
 $ input-emulator kbd key t
 $ input-emulator kbd keyup ctrl
 $ input-emulator kbd key q
 $ input-emulator stop kbd
```
#### 3.2.4 Status example
```
 $ input-emulator status
 Online devices:
   kbd: /sys/devices/virtual/input/input44
 mouse: /sys/devices/virtual/input/input46
 touch: /sys/devices/virtual/input/input45
```
#### 3.2.5 Run script example (Not working yet)
```
 $ input-emulator run examples/keyboard-test.lua
```

## 4. Installation from source

Install steps:

```
 $ meson build
 $ meson compile -C build
 $ meson install -C build
```

See meson_options.txt for tio specific build options.

Note: The meson install steps may differ depending on your specific system and
environment.

Note: To build successfully with scripting support you need liblua-dev installed (optional).

## 4. Contribute

Feel free to improve the implementation. It is open source and released under
the GPLv2 license.

## 5. Sponsors

 * DEIF A/S
