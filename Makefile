
all: keyboard-simulator

keyboard-simulator: keyboard-simulator.c keyboard-simulator.h script.c script.h
	gcc keyboard-simulator.c script.c -o keyboard-simulator

clean:
	rm -f *.o keyboard-simulator

.PHONY: clean
