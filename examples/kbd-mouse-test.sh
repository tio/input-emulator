#! /bin/bash

# Simple keyboard and mouse test

ie=input-emulator

${ie} start mouse --x-max 2560 --y-max 1440
${ie} start kbd
${ie} status
${ie} mouse button left
${ie} mouse move -10000 -10000
${ie} mouse move 1280 720
${ie} kbd type 'Hello World'
# ${ie} stop kbd
# ${ie} stop mouse
${ie} stop all
