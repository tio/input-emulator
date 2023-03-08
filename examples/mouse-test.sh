#! /bin/bash

# Simple mouse test

ie=input-emulator

${ie} start mouse --x-max 1920 --y-max 1080
${ie} status
${ie} mouse move -10000 -10000
${ie} mouse move 960 540
${ie} mouse button right
${ie} stop mouse
