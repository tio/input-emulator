#! /bin/bash

# Simple keyboard and mouse test

ie=input-emulator

${ie} start mouse --x-max 1920 --y-max 1080
${ie} start kbd
${ie} status
${ie} mouse button left
${ie} mouse move -10000 -10000
${ie} mouse move 960 540
${ie} kbd type "echo 'Hello World'"
${ie} kbd key enter
${ie} stop all
