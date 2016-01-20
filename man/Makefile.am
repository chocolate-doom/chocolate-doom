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
MANDIR = $(top_srcdir)/man
DOCGEN = $(MANDIR)/docgen

$(SETUP_MAN_PAGES): chocolate-setup.6
	cp chocolate-setup.6 $@

@PROGRAM_PREFIX@doom.6: ../src $(MANPAGE_GEN_FILES)
	$(DOCGEN) -g doom -m doom.template $(top_srcdir)/src $(top_srcdir)/src/doom > $@

default.cfg.5: $(top_srcdir)/src $(MANDIR)/default.cfg.template
	$(DOCGEN) -g doom -m $(MANDIR)/default.cfg.template \
                 -c default $(top_srcdir)/src/m_config.c > $@

@PROGRAM_PREFIX@doom.cfg.5: ../src extra.cfg.template
	$(DOCGEN) -g doom -m extra.cfg.template \
                 -c extended $(top_srcdir)/src/m_config.c > $@

CMDLINE.doom : $(MANDIR)/CMDLINE.template $(top_srcdir)/src $(top_srcdir)/src/doom
	$(DOCGEN) -p $(MANDIR)/CMDLINE.template $(top_srcdir)/src/ $(top_srcdir)/src/doom/ > $@

INSTALL.doom: $(MANDIR)/INSTALL.template
	$(MANDIR)/simplecpp -DDOOM -DPRECOMPILED < $(MANDIR)/INSTALL.template > $@


@PROGRAM_PREFIX@heretic.6: $(top_srcdir)/src $(MANPAGE_GEN_FILES)
	$(DOCGEN) -g heretic -m $(top_srcdir)/man/heretic.template $(top_srcdir)/src $(top_srcdir)/src/heretic > $@

heretic.cfg.5: $(top_srcdir)/src $(MANDIR)/default.cfg.template
	$(DOCGEN) -g heretic -m $(MANDIR)/default.cfg.template \
                 -c default $(top_srcdir)/src/m_config.c > $@

@PROGRAM_PREFIX@heretic.cfg.5: $(top_srcdir)/src $(MANDIR)/extra.cfg.template
	$(DOCGEN) -g heretic -m $(MANDIR)/extra.cfg.template \
                 -c extended $(top_srcdir)/src/m_config.c > $@

CMDLINE.heretic : $(MANDIR)/CMDLINE.template $(top_srcdir)/src $(top_srcdir)/src/heretic
	$(DOCGEN) -p $(MANDIR)/CMDLINE.template $(top_srcdir)/src/ $(top_srcdir)/src/heretic/ > $@

INSTALL.heretic: $(MANDIR)/INSTALL.template
	$(MANDIR)/simplecpp -DHERETIC -DPRECOMPILED < $(MANDIR)/INSTALL.template > $@


@PROGRAM_PREFIX@hexen.6: $(top_srcdir)/src $(MANPAGE_GEN_FILES)
	$(DOCGEN) -g hexen -m $(MANDIR)/hexen.template $(top_srcdir)/src $(top_srcdir)/src/hexen > $@

hexen.cfg.5: $(top_srcdir)/src $(MANDIR)/default.cfg.template
	$(DOCGEN) -g hexen -m $(MANDIR)/default.cfg.template \
                 -c default $(top_srcdir)/src/m_config.c > $@

@PROGRAM_PREFIX@hexen.cfg.5: $(top_srcdir)/src $(MANDIR)/extra.cfg.template
	$(DOCGEN) -g hexen -m $(MANDIR)/extra.cfg.template \
                 -c extended $(top_srcdir)/src/m_config.c > $@

CMDLINE.hexen : $(MANDIR)/CMDLINE.template $(top_srcdir)/src $(top_srcdir)/src/hexen
	$(DOCGEN) -p $(MANDIR)/CMDLINE.template $(top_srcdir)/src/ $(top_srcdir)/src/hexen/ > $@

INSTALL.hexen: $(MANDIR)/INSTALL.template
	$(MANDIR)/simplecpp -DHEXEN -DPRECOMPILED < $(MANDIR)/INSTALL.template > $@


@PROGRAM_PREFIX@strife.6: $(top_srcdir)/src $(MANPAGE_GEN_FILES)
	$(DOCGEN) -g strife -m $(MANDIR)/strife.template $(top_srcdir)/src $(top_srcdir)/src/strife > $@

strife.cfg.5: $(top_srcdir)/src $(MANDIR)/default.cfg.template
	$(DOCGEN) -g strife -m $(MANDIR)/default.cfg.template \
                 -c default $(top_srcdir)/src/m_config.c > $@

@PROGRAM_PREFIX@strife.cfg.5: $(top_srcdir)/src $(MANDIR)/extra.cfg.template
	$(DOCGEN) -g strife -m $(MANDIR)/extra.cfg.template \
                 -c extended $(top_srcdir)/src/m_config.c > $@

CMDLINE.strife : $(MANDIR)/CMDLINE.template $(top_srcdir)/src $(top_srcdir)/src/strife
	$(DOCGEN) -p $(MANDIR)/CMDLINE.template $(top_srcdir)/src/ $(top_srcdir)/src/strife/ > $@

INSTALL.strife: INSTALL.template
	$(MANDIR)/simplecpp -DSTRIFE -DPRECOMPILED < $(MANDIR)/INSTALL.template > $@


INSTALL: $(MANDIR)/INSTALL.template
	$(MANDIR)//simplecpp -DDOOM -DHERETIC -DHEXEN -DSTRIFE \
	            -DPRECOMPILED < $(MANDIR)/INSTALL.template > $@

endif

EXTRA_DIST = $(man_MANS) $(MANPAGE_GEN_FILES)        \
             wikipages                               \
             CMDLINE.template                        \
             INSTALL.template                        \
             simplecpp

