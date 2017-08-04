SUBDIRS = bash-completion

MANPAGE_GEN_FILES = environ.man           \
                    iwad_paths.man        \
                    doom.template         \
                    heretic.template      \
                    hexen.template        \
                    strife.template       \
                    docgen                \
                    default.cfg.template  \
                    extra.cfg.template

doomdocsdir = ${docdir}/../${PROGRAM_PREFIX}doom
hereticdocsdir = ${docdir}/../${PROGRAM_PREFIX}heretic
hexendocsdir = ${docdir}/../${PROGRAM_PREFIX}hexen
strifedocsdir = ${docdir}/../${PROGRAM_PREFIX}strife

if HAVE_PYTHON

GENERATED_MAN_PAGES =               \
           @PROGRAM_PREFIX@doom.6         \
           default.cfg.5            \
           @PROGRAM_PREFIX@doom.cfg.5     \
           @PROGRAM_PREFIX@heretic.6      \
           heretic.cfg.5            \
           @PROGRAM_PREFIX@heretic.cfg.5  \
           @PROGRAM_PREFIX@hexen.6        \
           hexen.cfg.5              \
           @PROGRAM_PREFIX@hexen.cfg.5    \
           @PROGRAM_PREFIX@strife.6       \
           strife.cfg.5             \
           @PROGRAM_PREFIX@strife.cfg.5

SETUP_MAN_PAGES =                          \
           @PROGRAM_PREFIX@doom-setup.6    \
           @PROGRAM_PREFIX@heretic-setup.6 \
           @PROGRAM_PREFIX@hexen-setup.6   \
           @PROGRAM_PREFIX@strife-setup.6

man_MANS = chocolate-server.6             \
           chocolate-setup.6              \
           $(GENERATED_MAN_PAGES)         \
           $(SETUP_MAN_PAGES)

doomdocs_DATA    = INSTALL.doom    CMDLINE.doom
hereticdocs_DATA = INSTALL.heretic CMDLINE.heretic
hexendocs_DATA   = INSTALL.hexen   CMDLINE.hexen
strifedocs_DATA  = INSTALL.strife  CMDLINE.strife

CLEANFILES = $(GENERATED_MAN_PAGES) $(SETUP_MAN_PAGES) \
             $(doomdocs_DATA) $(hereticdocs_DATA) \
             $(hexendocs_DATA) $(strifedocs_DATA)
DOCGEN = $(srcdir)/docgen

$(SETUP_MAN_PAGES): chocolate-setup.6
	cp $(srcdir)/chocolate-setup.6 $@

@PROGRAM_PREFIX@doom.6: $(top_srcdir)/src $(MANPAGE_GEN_FILES)
	$(DOCGEN) -g doom -m $(srcdir)/doom.template \
                  $(top_srcdir)/src $(top_srcdir)/src/doom > $@

default.cfg.5: $(top_srcdir)/src $(srcdir)/default.cfg.template
	$(DOCGEN) -g doom -m $(srcdir)/default.cfg.template \
                 -c default $(top_srcdir)/src/m_config.c > $@

@PROGRAM_PREFIX@doom.cfg.5: $(top_srcdir)/src extra.cfg.template
	$(DOCGEN) -g doom -m $(srcdir)/extra.cfg.template \
                 -c extended $(top_srcdir)/src/m_config.c > $@

CMDLINE.doom : CMDLINE.template $(top_srcdir)/src $(top_srcdir)/src/doom
	$(DOCGEN) -p $(srcdir)/CMDLINE.template \
                  $(top_srcdir)/src/ $(top_srcdir)/src/doom/ > $@

INSTALL.doom: INSTALL.template
	$(srcdir)/simplecpp -DDOOM -DPRECOMPILED < $(srcdir)/INSTALL.template > $@


@PROGRAM_PREFIX@heretic.6: $(top_srcdir)/src $(MANPAGE_GEN_FILES) heretic.template
	$(DOCGEN) -g heretic -m $(srcdir)/heretic.template \
                  $(top_srcdir)/src $(top_srcdir)/src/heretic > $@

heretic.cfg.5: $(top_srcdir)/src $(srcdir)/default.cfg.template
	$(DOCGEN) -g heretic -m $(srcdir)/default.cfg.template \
                  -c default $(top_srcdir)/src/m_config.c > $@

@PROGRAM_PREFIX@heretic.cfg.5: $(top_srcdir)/src extra.cfg.template
	$(DOCGEN) -g heretic -m $(srcdir)/extra.cfg.template \
                 -c extended $(top_srcdir)/src/m_config.c > $@

CMDLINE.heretic : CMDLINE.template $(top_srcdir)/src $(top_srcdir)/src/heretic
	$(DOCGEN) -p $(srcdir)/CMDLINE.template \
                  $(top_srcdir)/src/ $(top_srcdir)/src/heretic/ > $@

INSTALL.heretic: INSTALL.template
	$(srcdir)/simplecpp -DHERETIC -DPRECOMPILED < $(srcdir)/INSTALL.template > $@


@PROGRAM_PREFIX@hexen.6: $(top_srcdir)/src $(MANPAGE_GEN_FILES)
	$(DOCGEN) -g hexen -m $(srcdir)/hexen.template \
                  $(top_srcdir)/src $(top_srcdir)/src/hexen > $@

hexen.cfg.5: $(top_srcdir)/src default.cfg.template
	$(DOCGEN) -g hexen -m $(srcdir)/default.cfg.template \
                 -c default $(top_srcdir)/src/m_config.c > $@

@PROGRAM_PREFIX@hexen.cfg.5: $(top_srcdir)/src extra.cfg.template
	$(DOCGEN) -g hexen -m $(srcdir)/extra.cfg.template \
                 -c extended $(top_srcdir)/src/m_config.c > $@

CMDLINE.hexen : CMDLINE.template $(top_srcdir)/src $(top_srcdir)/src/hexen
	$(DOCGEN) -p $(srcdir)/CMDLINE.template \
                  $(top_srcdir)/src/ $(top_srcdir)/src/hexen/ > $@

INSTALL.hexen: INSTALL.template
	$(srcdir)/simplecpp -DHEXEN -DPRECOMPILED < $(srcdir)/INSTALL.template > $@


@PROGRAM_PREFIX@strife.6: $(top_srcdir)/src $(MANPAGE_GEN_FILES)
	$(DOCGEN) -g strife -m $(srcdir)/strife.template \
                  $(top_srcdir)/src $(top_srcdir)/src/strife > $@

strife.cfg.5: $(top_srcdir)/src default.cfg.template
	$(DOCGEN) -g strife -m $(srcdir)/default.cfg.template \
                 -c default $(top_srcdir)/src/m_config.c > $@

@PROGRAM_PREFIX@strife.cfg.5: $(top_srcdir)/src extra.cfg.template
	$(DOCGEN) -g strife -m $(srcdir)/extra.cfg.template \
                 -c extended $(top_srcdir)/src/m_config.c > $@

CMDLINE.strife : CMDLINE.template $(top_srcdir)/src $(top_srcdir)/src/strife
	$(DOCGEN) -p $(srcdir)/CMDLINE.template \
                  $(top_srcdir)/src/ $(top_srcdir)/src/strife/ > $@

INSTALL.strife: INSTALL.template
	$(srcdir)/simplecpp -DSTRIFE -DPRECOMPILED < $(srcdir)/INSTALL.template > $@


INSTALL: INSTALL.template
	$(srcdir)//simplecpp -DDOOM -DHERETIC -DHEXEN -DSTRIFE \
	            -DPRECOMPILED < $(srcdir)/INSTALL.template > $@

endif

EXTRA_DIST = $(man_MANS) $(MANPAGE_GEN_FILES)        \
             wikipages                               \
             CMDLINE.template                        \
             INSTALL.template                        \
             simplecpp

