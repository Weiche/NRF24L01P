CC = gcc
CFLAGS = -I.
OUTPUT = nrf24
HALDIR = hal
HALTYPE = NRF24_HAL_LINUX
SYMBOL = -D$(HALTYPE)
FILES = *.c $(HALDIR)/*.c

PARAMS = $(CFLAGS) -o $(OUTPUT) $(SYMBOL)
all:
	$(CC) $(PARAMS) $(FILES) $(MAIN)
debug:
	$(CC) $(PARAMS) -g $(FILES) $(MAIN)
clean:
	rm -f *.o $(OUTPUT)
