CC = gcc
CFLAGS = -I.
OUTPUT = nrf24
HALDIR = hal
FILES = *.c $(HALDIR)/*.c
all:
	$(CC) $(CFLAGS) -o $(OUTPUT) $(FILES)
debug:
	$(CC) $(CFLAGS) -g -o $(OUTPUT) $(FILES)
clean:
	rm -f *.o $(OUTPUT)
