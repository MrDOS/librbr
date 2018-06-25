#! /bin/sh

## \file shell-filter.sh
##
## \brief Doxygen filter for makefiles/shell scripts.
##
## \author Samuel Coleman <samuel.coleman@rbr-global.com>
## \copyright Copyright (c) 2017 RBR Ltd

# We want Doxygen to handle lines starting with “##” so we'll re-prefix with
# “//!”, which gets picked up by the C parser. And we want to capture
# definitions of makefile constants, so we'll rearrange those so they look like
# C macros. Then we filter out everything else so Doxygen doesn't get confused.

sed -e 's/^##/\/\/!/' \
    -e 's/\(.*\) \:= /#define \1 /' \
    "$1" \
    | grep '^$\|^//!\|#define'
