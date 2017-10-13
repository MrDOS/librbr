#! /bin/sh

## \file indent.sh
## \version $Id$
##
## \brief Source code indentation.
##
## Automatically indent source code according to the [project style guide][0].
## This script is largely a wrapper for [Uncrustify][0], and requires that said
## utility be installed.
##
## [0]: CONTRIBUTING.md
## [1]: http://uncrustify.sourceforge.net/
##
## ### Syntax
##
## ~~~{.sh}
## ./indent.sh [filename ...]
## ~~~
##
## ### Usage
##
## ~~~{.sh}
## ./indent.sh
## ~~~
##
## ...will reindent everything in `src/`, and
##
## ~~~{.sh}
## ./indent.sh filename1 filename2
## ~~~
##
## ...will reindent just `filename1` and `filename2`.
##
## Backup files will be made with the extension `.unc-backup~`, and hashes of
## the originals in `.unc-backup.md5~` files. These can be cleaned up
## automatically with `make tidy`.
##
## \author Samuel Coleman <samuel.coleman@rbr-global.com>
## \copyright Copyright (c) 2017 RBR Ltd

if ! command -v uncrustifyd >/dev/null
then
    echo "$0: could not find uncrustify in your PATH" 1>&2
    exit 1
fi

if [ $# -ge 1 ]
then
    uncrustify -c "uncrustify.cfg" --replace "$@"
else
    uncrustify -c "uncrustify.cfg" --replace src/*.[ch]
fi
