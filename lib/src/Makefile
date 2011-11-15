
CC=gcc
CFLAGS=-DBUILD_LIB -D_REENTRANT -I.. -W -Wall -Wextra -Os -fomit-frame-pointer -fno-strict-aliasing -fvisibility=hidden -fPIC
LDFLAGS=-lpthread -s
EXEC=../libngadmin.so

SRC=$(wildcard *.c)
OBJ=$(SRC:.c=.o)



$(EXEC): $(OBJ)
	$(CC) $^ -shared -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) -c $^ -o $@ $(CFLAGS)


clean:
	@rm -f *.o

mrproper: clean
	@rm -f $(EXEC)


