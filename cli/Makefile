
CC=gcc
CFLAGS=-I../lib -W -Wall -Wextra -Os -fomit-frame-pointer
LDFLAGS=-L../lib -lngadmin -lreadline -s
EXEC=admin

SRC=$(wildcard *.c)
OBJ=$(SRC:.c=.o)



$(EXEC): $(OBJ)
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) -c $^ -o $@ $(CFLAGS)


clean:
	@rm -f *.o

mrproper: clean
	@rm -f $(EXEC)



