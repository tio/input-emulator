#! /bin/bash

# Simple keyboard test

ie=input-emulator

${ie} start kbd

# Press ctrl-l to clear screen
${ie} kbd keydown ctrl
${ie} kbd key l
${ie} kbd keyup ctrl

# Say hello
${ie} kbd type "echo 'Hello World!'"
sleep 0.5
${ie} kbd key enter

${ie} stop kbd
