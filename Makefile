CC := gcc
OPTS := -std=c11 -Wall -Wextra -Wpedantic

default:
	$(CC) -O3 $(OPTS) src/errhandle.c src/io.c src/tokeniser.c src/parser.c src/queries.c src/main.c -o main

debug:
	$(CC) -Og -g $(OPTS) src/errhandle.c src/io.c src/tokeniser.c src/parser.c src/queries.c src/main.c -o main

clean:
	rm main
