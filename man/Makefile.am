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

if HAVE_WINDRES

WIN32=-D_WIN32

endif

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
           @PROGRAM_PREFIX@strife.cfg.5   \
           @PROGRAM_PREFIX@server.6

SETUP_MAN_PAGES =                          \
           @PROGRAM_PREFIX@doom-setup.6    \
           @PROGRAM_PREFIX@heretic-setup.6 \
           @PROGRAM_PREFIX@hexen-setup.6   \
           @PROGRAM_PREFIX@strife-setup.6

man_MANS = $(GENERATED_MAN_PAGES)         \
           $(SETUP_MAN_PAGES)

doomdocs_DATA    = INSTALL.doom    CMDLINE.doom
hereticdocs_DATA = INSTALL.heretic CMDLINE.heretic
hexendocs_DATA   = INSTALL.hexen   CMDLINE.hexen
strifedocs_DATA  = INSTALL.strife  CMDLINE.strife

CLEANFILES = $(GENERATED_MAN_PAGES) $(SETUP_MAN_PAGES) \
             $(doomdocs_DATA) $(hereticdocs_DATA) \
             $(hexendocs_DATA) $(strifedocs_DATA)
DOCGEN = $(srcdir)/docgen
DOCGEN_COMMON_ARGS = -n "@PROGRAM_SPREFIX@" -s "@PACKAGE_NAME@" -z "@PACKAGE_SHORTNAME@"

@PROGRAM_PREFIX@doom.6: $(top_srcdir)/src $(MANPAGE_GEN_FILES)
	$(DOCGEN) $(DOCGEN_COMMON_ARGS) \
                  -g doom -m $(srcdir)/doom.template \
                  $(top_srcdir)/src $(top_srcdir)/src/doom > $@

default.cfg.5: $(top_srcdir)/src $(srcdir)/default.cfg.template
	$(DOCGEN) $(DOCGEN_COMMON_ARGS) \
                  -g doom -m $(srcdir)/default.cfg.template \
                  -c default $(top_srcdir)/src/m_config.c > $@

@PROGRAM_PREFIX@doom.cfg.5: $(top_srcdir)/src extra.cfg.template
	$(DOCGEN) $(DOCGEN_COMMON_ARGS) \
                  -g doom -m $(srcdir)/extra.cfg.template \
                  -c extended $(top_srcdir)/src/m_config.c > $@

CMDLINE.doom : CMDLINE.template $(top_srcdir)/src $(top_srcdir)/src/doom
	$(DOCGEN) $(DOCGEN_COMMON_ARGS) \
                  -p $(srcdir)/CMDLINE.template \
                  $(top_srcdir)/src/ $(top_srcdir)/src/doom/ > $@

CMDLINE.doom.wikitext : $(top_srcdir)/src $(top_srcdir)/src/doom
	$(DOCGEN) $(DOCGEN_COMMON_ARGS) \
                  -w $(top_srcdir)/src/ $(top_srcdir)/src/doom/ > $@

INSTALL.doom: INSTALL.template
	$(srcdir)/simplecpp -DDOOM $(WIN32) \
            -DLONG_GAME_NAME="@PACKAGE_SHORTNAME@ Doom" \
            -DLONG_EXE_NAME="@PROGRAM_PREFIX@doom" \
            -DPRECOMPILED < $(srcdir)/INSTALL.template > $@

@PROGRAM_PREFIX@heretic.6: $(top_srcdir)/src $(MANPAGE_GEN_FILES) heretic.template
	$(DOCGEN) $(DOCGEN_COMMON_ARGS) \
                 -g heretic -m $(srcdir)/heretic.template \
                  $(top_srcdir)/src $(top_srcdir)/src/heretic > $@

heretic.cfg.5: $(top_srcdir)/src $(srcdir)/default.cfg.template
	$(DOCGEN) $(DOCGEN_COMMON_ARGS) \
                 -g heretic -m $(srcdir)/default.cfg.template \
                  -c default $(top_srcdir)/src/m_config.c > $@

@PROGRAM_PREFIX@heretic.cfg.5: $(top_srcdir)/src extra.cfg.template
	$(DOCGEN) $(DOCGEN_COMMON_ARGS) \
                 -g heretic -m $(srcdir)/extra.cfg.template \
                 -c extended $(top_srcdir)/src/m_config.c > $@

CMDLINE.heretic : CMDLINE.template $(top_srcdir)/src $(top_srcdir)/src/heretic
	$(DOCGEN) $(DOCGEN_COMMON_ARGS) \
                 -p $(srcdir)/CMDLINE.template \
                  $(top_srcdir)/src/ $(top_srcdir)/src/heretic/ > $@

INSTALL.heretic: INSTALL.template
	$(srcdir)/simplecpp -DHERETIC $(WIN32) \
            -DLONG_GAME_NAME="@PACKAGE_SHORTNAME@ Heretic" \
            -DLONG_EXE_NAME="@PROGRAM_PREFIX@heretic" \
            -DPRECOMPILED < $(srcdir)/INSTALL.template > $@


@PROGRAM_PREFIX@hexen.6: $(top_srcdir)/src $(MANPAGE_GEN_FILES)
	$(DOCGEN) $(DOCGEN_COMMON_ARGS) \
                 -g hexen -m $(srcdir)/hexen.template \
                  $(top_srcdir)/src $(top_srcdir)/src/hexen > $@

hexen.cfg.5: $(top_srcdir)/src default.cfg.template
	$(DOCGEN) $(DOCGEN_COMMON_ARGS) \
                 -g hexen -m $(srcdir)/default.cfg.template \
                 -c default $(top_srcdir)/src/m_config.c > $@

@PROGRAM_PREFIX@hexen.cfg.5: $(top_srcdir)/src extra.cfg.template
	$(DOCGEN) $(DOCGEN_COMMON_ARGS) \
                 -g hexen -m $(srcdir)/extra.cfg.template \
                 -c extended $(top_srcdir)/src/m_config.c > $@

CMDLINE.hexen : CMDLINE.template $(top_srcdir)/src $(top_srcdir)/src/hexen
	$(DOCGEN) $(DOCGEN_COMMON_ARGS) \
                  -p $(srcdir)/CMDLINE.template \
                  $(top_srcdir)/src/ $(top_srcdir)/src/hexen/ > $@

INSTALL.hexen: INSTALL.template
	$(srcdir)/simplecpp -DHEXEN $(WIN32) \
            -DLONG_GAME_NAME="@PACKAGE_SHORTNAME@ Hexen" \
            -DLONG_EXE_NAME="@PROGRAM_PREFIX@hexen" \
            -DPRECOMPILED < $(srcdir)/INSTALL.template > $@

@PROGRAM_PREFIX@strife.6: $(top_srcdir)/src $(MANPAGE_GEN_FILES)
	$(DOCGEN) $(DOCGEN_COMMON_ARGS) \
                  -g strife -m $(srcdir)/strife.template \
                  $(top_srcdir)/src $(top_srcdir)/src/strife > $@

@PROGRAM_PREFIX@server.6: $(top_srcdir)/src $(MANPAGE_GEN_FILES)
	$(DOCGEN) $(DOCGEN_COMMON_ARGS) \
                  -g server -m $(srcdir)/server.template \
                  $(top_srcdir)/src > $@

$(SETUP_MAN_PAGES): $(top_srcdir)/src $(MANPAGE_GEN_FILES)
	$(DOCGEN) $(DOCGEN_COMMON_ARGS) \
                  -g $(patsubst @PROGRAM_PREFIX@%-setup.6,%,$@) \
                  -m $(srcdir)/setup.template \
                  $(top_srcdir)/src > $@

strife.cfg.5: $(top_srcdir)/src default.cfg.template
	$(DOCGEN) $(DOCGEN_COMMON_ARGS) \
                  -g strife -m $(srcdir)/default.cfg.template \
                  -c default $(top_srcdir)/src/m_config.c > $@

@PROGRAM_PREFIX@strife.cfg.5: $(top_srcdir)/src extra.cfg.template
	$(DOCGEN) $(DOCGEN_COMMON_ARGS) \
                  -g strife -m $(srcdir)/extra.cfg.template \
                  -c extended $(top_srcdir)/src/m_config.c > $@

CMDLINE.strife : CMDLINE.template $(top_srcdir)/src $(top_srcdir)/src/strife
	$(DOCGEN) $(DOCGEN_COMMON_ARGS) \
                  -p $(srcdir)/CMDLINE.template \
                  $(top_srcdir)/src/ $(top_srcdir)/src/strife/ > $@

INSTALL.strife: INSTALL.template
	$(srcdir)/simplecpp -DSTRIFE $(WIN32) \
            -DLONG_EXE_NAME="@PROGRAM_PREFIX@strife" \
            -DLONG_GAME_NAME="@PACKAGE_SHORTNAME@ Strife" \
            -DPRECOMPILED < $(srcdir)/INSTALL.template > $@


INSTALL: INSTALL.template
	$(srcdir)//simplecpp -DDOOM -DHERETIC -DHEXEN -DSTRIFE \
            -DLONG_GAME_NAME="@PACKAGE_SHORTNAME@ Doom" \
            -DLONG_EXE_NAME="@PROGRAM_PREFIX@doom" \
            -DPRECOMPILED < $(srcdir)/INSTALL.template > $@

endif

EXTRA_DIST = $(man_MANS) $(MANPAGE_GEN_FILES)        \
             wikipages                               \
             CMDLINE.template                        \
             INSTALL.template                        \
             simplecpp

