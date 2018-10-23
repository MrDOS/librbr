## \file Makefile
##
## \brief Build automation.
##
## This makefile provides three different targets of interest to the end user:
##
## - `lib` will build the library (`bin/librbr.a`)
## - `docs` will generate the documentation via Doxygen (in `docs/`)
## - `tests` will run library tests (from `tests/`)
##
## Additional targets may be useful to developers:
##
## - `clean` will remove any compiled binaries and documentation
## - `devdocs` will generate the documentation inclusive of content only of
##   interest to library developers
## - `tidy` will remove indent backup files
##
## \copyright
## Copyright (c) 2018 RBR Ltd.
## Licensed under the Apache License, Version 2.0.

## \brief The project name.
##
## Embedded into the library and documentation.
##
## Project forks might like to override this by setting the `LIB_NAME`
## environment variable before invoking `make(1)` to easily identify which
## library variant is in use.
export LIB_NAME ?= librbr

## \brief The project version.
##
## Embedded into the library and documentation. Determined automatically from
## the Git tag.
export LIB_VERSION ?= $(shell \
  git describe --tags --dirty --always \
  | sed -e 's|^v||')

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
CFLAGS := -Werror \
          -Wall \
          -Wextra \
          -pedantic \
          -pedantic-errors \
          -std=c99 \
          -g

CFLAGS += -DLIB_NAME=\""$(LIB_NAME)"\" \
          -DLIB_VERSION=\""$(LIB_VERSION)"\" \
          -DLIB_BUILD_DATE=\"$(shell date '+%FT%T%z')\"

all: lib docs tests

lib: bin bin/librbr.a(src/RBRInstrument.o \
                      src/RBRInstrumentCommunication.o \
                      src/RBRInstrumentConfiguration.o \
                      src/RBRInstrumentDeployment.o \
                      src/RBRInstrumentFetching.o \
                      src/RBRInstrumentGating.o \
                      src/RBRInstrumentHardwareErrors.o \
                      src/RBRInstrumentInternal.o \
                      src/RBRInstrumentMemory.o \
                      src/RBRInstrumentOther.o \
                      src/RBRInstrumentSchedule.o \
                      src/RBRInstrumentSecurity.o \
                      src/RBRInstrumentStreaming.o \
                      src/RBRInstrumentVehicle.o)

.PHONY: docs
docs:
	doxygen Doxyfile-docs

.PHONY: devdocs
devdocs:
	doxygen Doxyfile-devdocs

tests: CFLAGS += -Isrc -Wno-error=unused-parameter
tests: LDFLAGS += -Lbin
tests: LDLIBS += -lrbr
tests: bin bin/tests
	./bin/tests

## \brief Test modules.
##
## Each one of these names corresponds to a C source file in the `tests/`
## directory. Tests declared within these files (using the `TEST_LOGGER2` and
## `TEST_LOGGER3` macros) are automatically discovered at build time and
## included in the test suite.
TEST_MODULES := communication \
                configuration \
                deployment \
                fetching \
                gating \
                memory \
                other \
                schedule \
                security \
                streaming \
                vehicle

bin/tests: tests/main.o \
           tests/tests.o \
           $(foreach module,$(TEST_MODULES),tests/$(module).o)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

tests/%.o: tests/%.c lib
	$(CC) $(CFLAGS) -c -o $@ $<

tests/tests.c: $(foreach module,$(TEST_MODULES),tests/$(module).c)
	@echo "/* This file is automatically generated. */" >$@
	@echo '#include "tests.h"' >>$@
	@grep -ho 'TEST_LOGGER[23]([A-Za-z_][A-Za-z0-9_]*)' $^ \
		| sed -e 's/$$/;/' >>$@
	@echo "TestDeclaration tests[] = {" >>$@
	@grep -ho 'TEST_LOGGER[23]([A-Za-z_][A-Za-z0-9_]*)' $^ \
		| sed -e 's/^TEST_LOGGER\([^(]*\)(\([^)]*\))/    {"\2", RBRINSTRUMENT_LOGGER\1, test_\2_l\1},/' \
		>>$@
	@echo "    {0}" >>$@
	@echo "};" >>$@

bin:
	mkdir bin

.PHONY: clean
clean:
	rm -Rf bin/ tests/tests.c tests/*.o docs/
