CFLAGS := -I../../src -Werror -Wall -Wextra -pedantic -pedantic-errors \
          -std=c99 -g
LDFLAGS := -L../../bin
LDLIBS := -lrbr

example: posix-download posix-fetch posix-stream posix-stream-sdl

posix-download: posix-shared.o posix-download.o

posix-fetch: posix-shared.o posix-fetch.o

posix-stream: posix-shared.o posix-stream.o

posix-stream-sdl: LDFLAGS += -lSDL2
posix-stream-sdl: posix-shared.o posix-stream-sdl.o

.PHONY: clean
clean:
	rm -Rf *.o posix-download posix-stream
