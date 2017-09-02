#!/bin/sh
set -e
./autogen.sh
make
make install DESTDIR=/tmp/whatever
make dist
