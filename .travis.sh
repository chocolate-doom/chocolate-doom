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
	make -j4
	make install DESTDIR=/tmp/whatever
	make dist
	make -j4 -C quickcheck check SOURCE_PORT=$PWD/src/crispy-doom
fi
