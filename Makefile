CC := gcc
OPTS := -Og -g -std=c11 -Wall -Wextra -Wpedantic

default:
	$(CC) $(OPTS) src/main.c -o main

clean:
	rm main
