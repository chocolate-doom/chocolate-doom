#!/bin/sh
set -e
if [ "$ANALYZE" = "true" ] ; then
	cppcheck --error-exitcode=1 -j2 -UTESTING -Iopl -Isrc -Isrc/setup opl pcsound src textscreen > /dev/null
else
	./autogen.sh --enable-werror
	make
	make install DESTDIR=/tmp/whatever
	make dist
fi
