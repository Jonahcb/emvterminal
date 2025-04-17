# Compiler and flags
CC = gcc
CFLAGS = -Wall
LDFLAGS = -framework PCSC

# Source files
SRC = src/main.c src/emv_terminal.c src/pcsc_driver.c src/tlv_parser.c src/apdu_builder.c

# Output binary
OUT = emvterminal

# Default target
all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

# Clean up binary
clean:
	rm -f $(OUT)

