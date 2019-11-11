#!/bin/sh
if [ "$ANALYZE" = "true" ] ; then
	cppcheck --error-exitcode=1 -j2 -UTESTING -Iopl -Isrc -Isrc/setup opl pcsound src textscreen 2> stderr.txt
	RET=$?
	if [ -s stderr.txt ]
	then
		cat stderr.txt
	fi
	exit $RET
else
	set -e
	./autogen.sh --enable-werror
	make
	make install DESTDIR=/tmp/whatever
	make dist
	make -C quickcheck check SOURCE_PORT=$PWD/src/chocolate-doom
fi
