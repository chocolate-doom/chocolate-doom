
MANPAGE_GEN_FILES=manpage.template docgen default.cfg.template extra.cfg.template

if HAVE_PYTHON

man_MANS=strawberry-doom.6 \
		strawberry-server.6 \
		strawberry-setup.6

strawberry-doom.6: ../src $(MANPAGE_GEN_FILES)
	./docgen -m manpage.template ../src > $@

default.cfg.5: ../src default.cfg.template
	./docgen -m default.cfg.template -c default.cfg ../src > $@

$(PACKAGE).cfg.5: ../src extra.cfg.template
	./docgen -m extra.cfg.template -c $(PACKAGE).cfg ../src > $@

endif

EXTRA_DIST = $(man_MANS) $(MANPAGE_GEN_FILES)        \
	     wikipages

