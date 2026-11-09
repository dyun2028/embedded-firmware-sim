CC ?= gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror -O1 -g
SRC = $(wildcard src/*.c)

test: tests/test_all.c $(SRC)
	$(CC) $(CFLAGS) -fsanitize=address,undefined -fno-sanitize-recover=all $^ -o build_test
	./build_test

clean:
	rm -f build_test
