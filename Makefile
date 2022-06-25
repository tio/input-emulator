
all: keyboard-simulator

keyboard-simulator: keyboard-simulator.c keyboard-simulator.h
	gcc keyboard-simulator.c -I/usr/include/lua5.4 -llua5.4 -o keyboard-simulator

clean:
	rm -f *.o keyboard-simulator

.PHONY: clean
