# keyboard-simulator - A simple keyboard simulator

## 1. Introduction

A very simple keyboard simulator which instructs the Linux kernel to create a
virtual keyboard input device through which one can script key presses.

The implementation is currently hardcoded for keyboards with Nordic layout but
it can easily be customized to fit any keyboard layout by customizing the key
mapping array.

It is made specifically for creating the animated GIF showing tio in action.

## 2. How to build

To build successfully you need liblua5.4-dev installed.

```
 $ make
```

## 3. How to run

```
 $ ./keyboard-simulator test.keyboard &
```

## 4. Contribute

Feel free to improve the implementation.
