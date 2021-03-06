#! /bin/sh

## \file shell-filter.sh
##
## \brief Doxygen filter for makefiles/shell scripts.
##
## \copyright
## Copyright (c) 2018 RBR Ltd.
## Licensed under the Apache License, Version 2.0.

# We want Doxygen to handle lines starting with “##” so we'll re-prefix with
# “//!”, which gets picked up by the C parser. And we want to capture
# definitions of makefile constants, so we'll rearrange those so they look like
# C macros. Then we filter out everything else so Doxygen doesn't get confused.

sed -e 's/^##/\/\/!/' \
    -e 's/^\(export\s*\)\?\([A-Za-z0-9_]*\)\s*[:?]\?= /#define \2 /' \
    -e ':c /\\$/ { N; s/\\\n\s*//g ; bc }' \
    "$1" \
    | grep '^$\|^//!\|^#define'
