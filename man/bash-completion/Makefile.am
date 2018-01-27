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

DOCGEN = $(top_srcdir)/man/docgen
DOCGEN_COMMON_ARGS = -n "@PROGRAM_SPREFIX@" -s "@PACKAGE_NAME@" -z "@PACKAGE_SHORTNAME@"

@PROGRAM_PREFIX@doom: $(top_srcdir)/src $(DOCGEN) $(BASH_COMPLETION_TEMPLATES)
	$(DOCGEN) $(DOCGEN_COMMON_ARGS) \
                  -g doom -b doom.template \
                  $(top_srcdir)/src $(top_srcdir)/src/doom > $@

@PROGRAM_PREFIX@heretic: $(top_srcdir)/src $(DOCGEN) $(BASH_COMPLETION_TEMPLATES)
	$(DOCGEN) $(DOCGEN_COMMON_ARGS) \
                  -g heretic -b heretic.template \
                  $(top_srcdir)/src $(top_srcdir)/src/heretic > $@

@PROGRAM_PREFIX@hexen: $(top_srcdir)/src $(DOCGEN) $(BASH_COMPLETION_TEMPLATES)
	$(DOCGEN) $(DOCGEN_COMMON_ARGS) \
                  -g hexen -b hexen.template \
                  $(top_srcdir)/src $(top_srcdir)/src/hexen > $@

@PROGRAM_PREFIX@strife: $(top_srcdir)/src $(DOCGEN) $(BASH_COMPLETION_TEMPLATES)
	$(DOCGEN) $(DOCGEN_COMMON_ARGS) \
                  -g strife -b strife.template \
                  $(top_srcdir)/src $(top_srcdir)/src/strife > $@

EXTRA_DIST = \
    $(BASH_COMPLETION_TEMPLATES) \
    $(BASH_COMPLETION_SCRIPTLETS)

else

EXTRA_DIST = \
    $(BASH_COMPLETION_TEMPLATES)

endif
