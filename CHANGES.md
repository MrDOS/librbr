# Changes

## v1.0.5

Release TBD.

### Fixed

* Fixed uninitialized variable in POSIX examples.

## v1.0.4

Released 2019-02-15.

### Fixed

* Fixed compatibility error with Logger2 instruments
  in `RBRInstrument_setClock()`.

## v1.0.3

Released 2019-01-29.

### Changed

* Moved public headers out of the `src/` directory
  into `include/`.
  This helps enforce the distinction
  between public and internal APIs
  and makes it slightly easier
  for consumer projects
  to include the library headers.

## v1.0.2

Released 2019-01-03.

### Changed

* Rebrand slightly from librbr to libRBR.
  This is reflected
  by the name of the library archive,
  which has changed
  from `librbr.a`
  to `libRBR.a`
  (and subsequently,
  must now be linked with `-lRBR`
  instead of `-lrbr`).

## v1.0.1

Released 2018-12-06.

### Added

* To the readme:
    * Added a short example of what user code might look like.
    * Added firmware support list.

### Fixed

* Added release date for v1.0.0 to this changelog.

## v1.0.0

Released 2018-12-05.

The initial library release version.
