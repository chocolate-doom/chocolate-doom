#!/bin/sh
EXTERN="$(grep -e 'extern[^A-Za-z0-9_]' $(find . -name *.c))"
if [ -n "$EXTERN" ] ; then
	echo "extern found in .c files:\n${EXTERN}"
	exit 1
fi
