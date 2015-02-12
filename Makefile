CC = gcc
INCLUDE = -I.
OUTPUT = nrf24
HAL_DIR = hal
HAL_TYPE = NRF24_HAL_LINUX
SYMBOL = -D$(HAL_TYPE)
FILES = *.c $(HAL_DIR)/*.c
OPT = 

PARAMS = $(INCLUDE) -o $(OUTPUT) $(SYMBOL) -O$(OPT)
all:
	$(CC) $(PARAMS) $(FILES) $(MAIN)
debug:
	$(CC) $(PARAMS) -g $(FILES) $(MAIN)
clean:
	rm -f *.o $(OUTPUT)
example:
	$(CC) $(PARAMS) $(FILES) example/receive_test.c