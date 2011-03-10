#!/bin/sh

mkdir autotools

aclocal
autoheader
automake
autoconf
automake

./configure "$@"

