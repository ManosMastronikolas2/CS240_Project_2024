CC=gcc
CFLAGS=-Wall -g
TARGET=cs240Elections
SRC=$(wildcard *.c)
HEADERS=$(wildcard *.h)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

.PHONY: clean

clean:
	rm -f $(TARGET)
