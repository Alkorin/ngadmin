
CC=gcc
CFLAGS=-DBUILD_LIB -I.. -W -Wall -Wextra -Os -fno-strict-aliasing -fvisibility=hidden -fPIC
LDFLAGS=

SRC=$(wildcard *.c)
OBJ=$(SRC:.c=.o)


ifeq ($(DEBUG), yes)
EXEC=../libngadmind.so
CFLAGS+=-g
LDFLAGS+=-g
else
EXEC=../libngadmin.so
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
	@rm -f ../libngadmin.so ../libngadmind.so


