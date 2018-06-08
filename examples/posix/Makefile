CFLAGS := -I../../src -Werror -Wall -Wextra -pedantic -pedantic-errors \
          -std=c99 -g
LDFLAGS := -L../../bin
LDLIBS := -lrbr

example: librbr-posix-example

.PHONY: clean
clean:
	rm -Rf librbr-posix-example
