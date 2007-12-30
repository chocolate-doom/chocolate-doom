
MANPAGE_GEN_FILES=header footer environment docgen

if HAVE_PYTHON

man_MANS=chocolate-doom.6

chocolate-doom.6: ../src $(MANPAGE_GEN_FILES)
	./docgen -m ../src > $@

endif

EXTRA_DIST = $(man_MANS) $(MANPAGE_GEN_FILES)        \
	     wikipages

