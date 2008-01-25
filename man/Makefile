
chocolate-doom.6: chocolate-doom-options.in head foot Makefile doom-options.in
	cp head $@
	echo .SH DOOM OPTIONS >> $@
	awk -F"\t" '{print ".TP\n\\fB"$$1"\\fR"; $$1=""; print }' < doom-options.in >>$@
	echo .SH CHOCOLATE\\-DOOM OPTIONS >> $@
	awk -F"\t" '{print ".TP\n\\fB"$$1"\\fR"; $$1=""; print }' < chocolate-doom-options.in >>$@
	m4 -DAUTHORS="`cat ../AUTHORS`" foot >> $@
