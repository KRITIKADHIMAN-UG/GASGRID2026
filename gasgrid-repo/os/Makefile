CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra -Wno-misleading-indentation -std=c99
SRC := $(wildcard src/*.c)
BIN := $(patsubst src/%.c,bin/%,$(SRC))
all: $(BIN)
bin/%: src/%.c
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $<
test: all
	sh tests/test_engine.sh
clean:
	rm -rf bin
