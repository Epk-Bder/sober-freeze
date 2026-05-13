CC = gcc
CFLAGS = -O2 -std=c11

TARGET = sober_freeze
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)