
CFLAGS=-W -Wall -Wextra -Os -fomit-frame-pointer -fno-strict-aliasing -pthread
LDFLAGS=-s -pthread
CC=gcc



all: admin emu
	


admin: admin.o protocol.o list.o command.o network.o controller.o
	$(CC) $^ -o $@ $(LDFLAGS)


emu: emu.o protocol.o list.o command.o
	$(CC) $^ -o $@ $(LDFLAGS)


%.o: %.c %.h
	$(CC) -c $< -o $@ $(CFLAGS)


%.o: %.c
	$(CC) -c $^ -o $@ $(CFLAGS)


.PHONY: clean mrproper
	


clean: 
	@rm -f *.o


mrproper: clean
	@rm -f admin emu



