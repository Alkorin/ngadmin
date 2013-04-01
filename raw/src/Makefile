
CC=gcc
AR=ar
CFLAGS=-I../include/ -I../../lib/include/ -W -Wall -Wextra -Os -fno-strict-aliasing -fvisibility=hidden -fPIC
EXEC=../librawnsdp.a

SRC=$(wildcard *.c)
OBJ=$(SRC:.c=.o)


ifeq ($(DEBUG), yes)
CFLAGS+=-g
else
CFLAGS+=-fomit-frame-pointer
endif


$(EXEC): $(OBJ)
	$(AR) rcs $@ $^

%.o: %.c
	$(CC) -c $^ -o $@ $(CFLAGS)


clean:
	@rm -f *.o

mrproper: clean
	@rm -f $(EXEC)


