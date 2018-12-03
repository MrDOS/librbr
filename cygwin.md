# Cygwin

librbr can be built and run on Windows under Cygwin.

## Prerequisites

In addition to the base Cygwin install,
you'll need to install these additional packages
(and their dependencies):

* `doxygen`
* `gcc-core` or `clang`
* `git`
* `make`

`libSDL2-devel` is optionally required
to successfully compile the `posix-stream-sdl` example,
but that example does not launch under Cygwin.
