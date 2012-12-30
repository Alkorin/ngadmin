
CC=gcc
CFLAGS=-I../lib -W -Wall -Wextra -Os
LDFLAGS=-L../lib -lreadline

SRC=$(wildcard *.c)
OBJ=$(SRC:.c=.o)


ifeq ($(DEBUG), yes)
EXEC=admind
CFLAGS+=-g
LDFLAGS+=-lngadmind -g
else
EXEC=admin
CFLAGS+=-fomit-frame-pointer
LDFLAGS+=-lngadmin -s
endif


$(EXEC): $(OBJ)
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) -c $^ -o $@ $(CFLAGS)


clean:
	@rm -f *.o

mrproper: clean
	@rm -f $(EXEC)



