
MANPAGE_GEN_FILES=header footer docgen

man_MANS=chocolate-doom.6

EXTRA_DIST = $(man_MANS) $(MANPAGE_GEN_FILES) 


if HAVE_PYTHON

chocolate-doom.6: ../src $(MANPAGE_GEN_FILES)
	./docgen -m ../src > $@

endif

