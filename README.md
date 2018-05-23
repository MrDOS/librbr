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

You may prefer to integrate
the entire library source
into your codebase.
In that case,
we strongly suggest doing so
via a [Git submodule]
where possible
to make updating easier.

[Makefile]: Makefile.html
[Git submodule]: https://git-scm.com/docs/git-submodule

## Using

* TODO: API introduction.

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
of the Apache License 2.0;
see https://www.apache.org/licenses/LICENSE-2.0.
