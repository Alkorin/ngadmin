
CC=gcc
CFLAGS=-I../lib/include/ -W -Wall -Wextra -Os
LDFLAGS=-L../lib -lngadmin -lreadline
EXEC=admin

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
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) -c $^ -o $@ $(CFLAGS)


clean:
	@rm -f *.o

mrproper: clean
	@rm -f $(EXEC)



