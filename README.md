# libRBR

## Introduction

libRBR provides an interface
for simplified communication
with RBR instruments.
The library isolates the user
from the low-level details
of instrument communication
by wrapping each instrument command
in a function with fully typed arguments.
Familiarity with the [instrument command set]
is still required
in order to know which commands to send,
but the library handles the intricacies
of waking the instrument,
response parsing,
etc.

For example:

~~~{.c}
RBRInstrumentSampling sampling;
RBRInstrument_getSampling(instrument, &sampling);
printf("The instrument is performing %s sampling every %" PRIi32 "ms.\n",
       RBRInstrumentSamplingMode_name(sampling.mode),
       sampling.period);
~~~

The library also offers basic parsing
for [EasyParse] sample data and events.

The library tries to be platform-agnostic.
It targets C99
and should be compilable
on any compliant compiler.

If you happen to have stumbled across this file
in a source tree somewhere,
you might be interested to know
that this project is maintained
[on Bitbucket].

[instrument command set]: https://docs.rbr-global.com/L3commandreference
[EasyParse]: https://docs.rbr-global.com/L3commandreference/format-of-stored-data/overview/easyparse-format
[on Bitbucket]: https://bitbucket.org/rbr/librbr

## Support

The library is intended to support
the following firmware types and versions.
The library should have good forwards compatibility
as breaking changes between firmware versions
are rare and made only when absolutely necessary.

| Firmware Type           | Generation | Version |
| ----------------------- | ---------- | ------- |
| 103 (Logger2, standard) | Early 2015 |  v1.440 |
| 104 (Logger3, standard) |  Late 2017 |  v1.094 |

## Building

The library can be built with GNU Make.
For documentation on Make targets,
see the [Makefile].

Library compilation requires a C99-compliant C compiler;
The library makes a few assumptions
about its host platform.
For details, see [the documentation on porting][porting].

API document compilation requires [Doxygen].

In most cases,
the library can be built
in a few steps.
First, check out the code with Git:

~~~{.sh}
git clone https://bitbucket.org/rbr/librbr.git
cd librbr
~~~

Then use `make(1)` to build the library:

~~~{.sh}
# Build just the library.
make lib
# Build and execute tests. Also builds the library if necessary.
make tests
# Build Doxygen documentation.
make docs
# Does all of the above.
make
~~~

Platform-specific instructions and advice
are available:

* [Cygwin]

[Makefile]: Makefile.html
[porting]: porting.md
[Doxygen]: http://doxygen.org/
[Cygwin]: cygwin.md

## Using

See [the introduction]
for an overview of library conventions.

API documentation is built into the `docs/` subdirectory.
Prebuilt API documentation corresponding to the latest release
is available at https://docs.rbr-global.com/librbr/.

For examples,
please see the `examples/` subdirectory.

You may prefer to integrate
the entire library source
into your codebase
rather than link against the library.
In that case,
we strongly suggest doing so
via a [Git submodule]
where possible
to make updating easier.

[the introduction]: introduction.md
[Git submodule]: https://git-scm.com/docs/git-submodule

## Contributing

The library is primarily maintained by RBR
and development is directed by our needs
and the needs of our [OEM] customers.
However, we're happy to take [contributions] generally.

[OEM]: https://rbr-global.com/products/oem
[contributions]: CONTRIBUTING.md

## License

This project is licensed under the terms
of the Apache License, Version 2.0;
see https://www.apache.org/licenses/LICENSE-2.0.

* The license is not “viral”.
  You can include it
  either as source
  or by linking against it,
  statically or dynamically,
  without affecting the licensing
  of your own code.
* You do not need to include RBR's copyright notice
  in your documentation,
  nor do you need to display it
  at program runtime.
  You must retain RBR's copyright notice
  in library source files.
* You are under no legal obligation
  to share your own modifications
  (although we would appreciate it
  if you did so).
* If you make changes to the source,
  in addition to retaining RBR's copyright notice,
  you must add a notice stating that you changed it.
  You may add your own copyright notices.
