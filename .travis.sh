#!/bin/sh
if [ "$ANALYZE" = "true" ] ; then
	# -D__GNUC__ is required for cppcheck to know about noreturn functions
	cppcheck --error-exitcode=1 -j2 -UTESTING -D__GNUC__ -Iopl -Isrc -Isrc/setup opl pcsound src textscreen 2> stderr.txt
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
	PREFIX=`sed -n '/PROGRAM_PREFIX/p' ${PWD}/config.h | cut -d '"' -f 2`
	make -j4 -C quickcheck check SOURCE_PORT=$PWD/src/${PREFIX}doom
fi
