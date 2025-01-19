CC := gcc
OPTS := -Og -g -std=c11 -Wall -Wextra -Wpedantic

default:
	$(CC) $(OPTS) src/errhandle.c src/io.c src/tokeniser.c src/parser.c src/queries.c src/main.c -o main

clean:
	rm main
