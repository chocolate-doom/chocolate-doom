bashcompletiondir=@bashcompletiondir@

BASH_COMPLETION_TEMPLATES = \
    doom.template \
    heretic.template \
    hexen.template \
    strife.template

if HAVE_PYTHON

BASH_COMPLETION_SCRIPTLETS = \
    @PROGRAM_PREFIX@doom \
    @PROGRAM_PREFIX@heretic \
    @PROGRAM_PREFIX@hexen \
    @PROGRAM_PREFIX@strife

bashcompletion_DATA = $(BASH_COMPLETION_SCRIPTLETS)
CLEANFILES = $(BASH_COMPLETION_SCRIPTLETS)

MANDIR = $(top_srcdir)/man
DOCGEN = $(MANDIR)/docgen

@PROGRAM_PREFIX@doom: $(top_srcdir)/src $(DOCGEN) $(BASH_COMPLETION_TEMPLATES)
	$(DOCGEN) -g doom -b $(MANDIR)/doom.template \
                  $(top_srcdir)/src $(top_srcdir)/src/doom > $@

@PROGRAM_PREFIX@heretic: $(top_srcdir)/src $(DOCGEN) $(BASH_COMPLETION_TEMPLATES)
	$(DOCGEN) -g heretic -b $(MANDIR)/heretic.template \
                  $(top_srcdir)/src $(top_srcdir)/src/heretic > $@

@PROGRAM_PREFIX@hexen: $(top_srcdir)/src $(DOCGEN) $(BASH_COMPLETION_TEMPLATES)
	$(DOCGEN) -g hexen -b $(MANDIR)/hexen.template \
                  $(top_srcdir)/src $(top_srcdir)/src/hexen > $@

@PROGRAM_PREFIX@strife: $(top_srcdir)/src $(DOCGEN) $(BASH_COMPLETION_TEMPLATES)
	$(DOCGEN) -g strife -b $(MANDIR)/strife.template \
                  $(top_srcdir)/src $(top_srcdir)/src/strife > $@

EXTRA_DIST = \
    $(BASH_COMPLETION_TEMPLATES) \
    $(BASH_COMPLETION_SCRIPTLETS)

else

EXTRA_DIST = \
    $(BASH_COMPLETION_TEMPLATES)

endif
