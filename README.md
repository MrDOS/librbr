# librbr

## Introduction

librbr provides an interface
for simplified communication
with RBR instruments.

The library tries to be platform-agnostic.
It targets C99
and should be compilable
on any compliant compiler.

If you happen to have stumbled across this file
in a source tree somewhere,
you might be interested to know
that this project is maintained
[on Bitbucket].

[on Bitbucket]: https://bitbucket.org/rbr/librbr

## Building

The library can be built with GNU Make.
For documentation on Make targets,
see the [Makefile].
Library compilation requires a C99-compliant C compiler;
API document compilation requires [Doxygen].

TODO: Platform assumptions/porting considerations.

Platform-specific instructions and advice
are available:

* [Cygwin](cygwin.md)

You may prefer to integrate
the entire library source
into your codebase.
In that case,
we strongly suggest doing so
via a [Git submodule]
where possible
to make updating easier.

[Makefile]: Makefile.html
[Doxygen]: http://doxygen.org/
[Git submodule]: https://git-scm.com/docs/git-submodule

## Using

API documentation is built into the `docs/` subdirectory.
Prebuilt API documentation corresponding to the latest release
is available at https://docs.rbr-global.com/librbr/.

* TODO: API introduction.
* TODO: Streaming data introduction.
* TODO: Data parsing introduction.

For examples,
please see the `examples/` subdirectory.

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
