
MANPAGE_GEN_FILES=chocolate-doom-options.in head foot Makefile doom-options.in

man_MANS=chocolate-doom.6

EXTRA_DIST = $(man_MANS) $(MANPAGE_GEN_FILES)

chocolate-doom.6: $(MANPAGE_GEN_FILES)
	cp head $@
	echo .SH DOOM OPTIONS >> $@
	awk -F"\t" '{print ".TP\n\\fB"$$1"\\fR"; $$1=""; print }' < doom-options.in >>$@
	echo .SH CHOCOLATE DOOM OPTIONS >> $@
	awk -F"\t" '{print ".TP\n\\fB"$$1"\\fR"; $$1=""; print }' < chocolate-doom-options.in >>$@
	sed "s/AUTHORS/`cat ../AUTHORS`/" < foot >> $@
