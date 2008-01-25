
MANPAGE_GEN_FILES=header footer environment docgen

if HAVE_PYTHON

man_MANS=strawberry-doom.6 strawberry-server.6 strawberry-setup.6

strawberry-doom.6: ../src $(MANPAGE_GEN_FILES)
	./docgen -m ../src > $@

endif

EXTRA_DIST = $(man_MANS) $(MANPAGE_GEN_FILES)        \
	     wikipages

