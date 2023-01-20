#! /bin/bash

ie=input-emulator

${ie} start kbd
${ie} kbd type "Hello World"
${ie} stop kbd
