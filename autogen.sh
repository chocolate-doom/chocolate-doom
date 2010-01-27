#!/bin/sh

mkdir autotools

aclocal
autoheader
automake -a -c
autoconf
automake

./configure "$@"

