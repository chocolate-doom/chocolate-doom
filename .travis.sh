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
	# This is a hack that hides warnings due to the boolean enum; 'false' and 'true'
	# are reserved words in C++ and cause -Wc++-compat to produce an enormous number
	# of warnings.
	sed -i 's/\bfalse\b/zzz_false/g; s/\btrue\b/zzz_true/g' $(find . -name '*.[ch]')
	set -e
	./autogen.sh --enable-werror
	make -j4
	make install DESTDIR=/tmp/whatever
	make dist
	PREFIX=`sed -n '/PROGRAM_PREFIX/p' ${PWD}/config.h | cut -d '"' -f 2`
	make -j4 -C quickcheck check SOURCE_PORT=$PWD/src/${PREFIX}doom
fi
