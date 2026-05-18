CFLAGS = -O2 -std=c11

all: sober_freeze

sober_freeze: main.c
	gcc $(CFLAGS) main.c -o sober_freeze

clean:
	rm -f sober_freeze
