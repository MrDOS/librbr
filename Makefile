## \file Makefile
##
## \brief Build automation.
##
## This makefile provides three different targets of interest to the end user:
##
## - `lib` will build the library (`bin/librbr.a`)
## - `docs` will generate the documentation via Doxygen (in `docs/`)
## - `test` will run library tests (from `tests/`)
##
## An additional target may be useful to developers:
##
## - `clean` will remove any compiled binaries and documentation
## - `tidy` will remove indent backup files
##
## \author Samuel Coleman <samuel.coleman@rbr-global.com>
## \copyright Copyright (c) 2017 RBR Ltd

## \brief Archiver flags.
##
## “Archive”, in this case, refers to the `.a` archive produced by building the
## library.
##
## - `c`: create the archive if necessary
## - `r`: replace existing contents of archive
## - `s`: create/update archive index
## - `U`: maintain original UID/GID/timestamp/mode of archive contents
##
## `U` is the default behaviour of the utility except on recent Debian (and
## therefore, Ubuntu) systems, so we specify it for safety.
ARFLAGS := crsU

## \brief C compilation flags.
##
## Conventionally, extensive warnings and warnings-as-errors would only be
## enabled for debug/development builds to broaden compiler compatibility.
## (Some compilers might generate warnings for different things than others,
## and with `-Werror`, that might prevent compilation.) However, to help ensure
## high code quality everywhere, we're leaving them all on by default. Please
## let us know if you encounter issues: we want to fix them.
##
## We're also turning on debug symbol generation by default so you don't have
## to go out of your way to enable them. If you don't want them, we suggest
## using `strip(1)` on the archives as opposed to changing this makefile to
## minimize the friction of pulling in library updates.
CFLAGS := -Werror -Wall -Wextra -pedantic -pedantic-errors -std=c99 -g

all: lib docs test

lib: bin bin/librbr.a(src/RBRInstrument.o \
                      src/RBRInstrumentCommunication.o \
                      src/RBRInstrumentFetching.o \
                      src/RBRInstrumentInternal.o \
                      src/RBRInstrumentMemory.o \
                      src/RBRInstrumentOther.o \
                      src/RBRInstrumentSchedule.o \
                      src/RBRInstrumentSecurity.o \
                      src/RBRInstrumentStreaming.o)

bin:
	mkdir bin

.PHONY: docs
docs:
	doxygen Doxyfile

test: lib
	@echo WARNING: Automated tests are unimplemented.

.PHONY: clean
clean:
	rm -Rf bin/ docs/
