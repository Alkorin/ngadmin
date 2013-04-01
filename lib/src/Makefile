
CC=gcc
CFLAGS=-DBUILD_LIB -I../../raw/include/ -I../include/ -W -Wall -Wextra -Os -fno-strict-aliasing -fvisibility=hidden -fPIC
LDFLAGS=-L../../raw/ -lrawnsdp
EXEC=../libngadmin.so

SRC=$(wildcard *.c)
OBJ=$(SRC:.c=.o)


ifeq ($(DEBUG), yes)
CFLAGS+=-g
LDFLAGS+=-g
else
CFLAGS+=-fomit-frame-pointer
LDFLAGS+=-s
endif


$(EXEC): $(OBJ)
	$(CC) $^ -shared -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) -c $^ -o $@ $(CFLAGS)


clean:
	@rm -f *.o

mrproper: clean
	@rm -f $(EXEC)


