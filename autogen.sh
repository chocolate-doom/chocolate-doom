#!/bin/sh

mkdir -p autotools

aclocal
autoheader
automake -ac
autoconf
automake

./configure "$@"
