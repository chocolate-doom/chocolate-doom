
MANPAGE_GEN_FILES=header footer 

man_MANS=chocolate-doom.6

EXTRA_DIST = $(man_MANS) $(MANPAGE_GEN_FILES)

chocolate-doom.6: $(MANPAGE_GEN_FILES)
	./docgen -m > $@

