#! /bin/bash

# Mouse stress test

ie=input-emulator
delay=0.5

${ie} start mouse --x-max 1920 --y-max 1080

${ie} mouse move -10000 -10000
${ie} mouse move 960 540

for (( c=1; c<=10; c++ ))
do
    ${ie} mouse scroll 1
    sleep ${delay}
    ${ie} mouse scroll -1
    sleep ${delay}
done

${ie} stop all
