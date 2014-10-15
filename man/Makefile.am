MANPAGE_GEN_FILES = environ.man           \
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

$(SETUP_MAN_PAGES): chocolate-setup.6
	cp $< $@

@PROGRAM_PREFIX@doom.6: ../src $(MANPAGE_GEN_FILES)
	./docgen -g doom -m doom.template ../src ../src/doom > $@

default.cfg.5: ../src default.cfg.template
	./docgen -g doom -m default.cfg.template \
                 -c default ../src/m_config.c > $@

@PROGRAM_PREFIX@doom.cfg.5: ../src extra.cfg.template
	./docgen -g doom -m extra.cfg.template \
                 -c extended ../src/m_config.c > $@

CMDLINE.doom : CMDLINE.template ../src ../src/doom
	./docgen -p CMDLINE.template ../src/ ../src/doom/ > $@

INSTALL.doom: INSTALL.template
	./simplecpp -DDOOM -DPRECOMPILED < INSTALL.template > $@


@PROGRAM_PREFIX@heretic.6: ../src $(MANPAGE_GEN_FILES)
	./docgen -g heretic -m heretic.template ../src ../src/heretic > $@

heretic.cfg.5: ../src default.cfg.template
	./docgen -g heretic -m default.cfg.template \
                 -c default ../src/m_config.c > $@

@PROGRAM_PREFIX@heretic.cfg.5: ../src extra.cfg.template
	./docgen -g heretic -m extra.cfg.template \
                 -c extended ../src/m_config.c > $@

CMDLINE.heretic : CMDLINE.template ../src ../src/heretic
	./docgen -p CMDLINE.template ../src/ ../src/heretic/ > $@

INSTALL.heretic: INSTALL.template
	./simplecpp -DHERETIC -DPRECOMPILED < INSTALL.template > $@


@PROGRAM_PREFIX@hexen.6: ../src $(MANPAGE_GEN_FILES)
	./docgen -g hexen -m hexen.template ../src ../src/hexen > $@

hexen.cfg.5: ../src default.cfg.template
	./docgen -g hexen -m default.cfg.template \
                 -c default ../src/m_config.c > $@

@PROGRAM_PREFIX@hexen.cfg.5: ../src extra.cfg.template
	./docgen -g hexen -m extra.cfg.template \
                 -c extended ../src/m_config.c > $@

CMDLINE.hexen : CMDLINE.template ../src ../src/hexen
	./docgen -p CMDLINE.template ../src/ ../src/hexen/ > $@

INSTALL.hexen: INSTALL.template
	./simplecpp -DHEXEN -DPRECOMPILED < INSTALL.template > $@


@PROGRAM_PREFIX@strife.6: ../src $(MANPAGE_GEN_FILES)
	./docgen -g strife -m strife.template ../src ../src/strife > $@

strife.cfg.5: ../src default.cfg.template
	./docgen -g strife -m default.cfg.template \
                 -c default ../src/m_config.c > $@

@PROGRAM_PREFIX@strife.cfg.5: ../src extra.cfg.template
	./docgen -g strife -m extra.cfg.template \
                 -c extended ../src/m_config.c > $@

CMDLINE.strife : CMDLINE.template ../src ../src/strife
	./docgen -p CMDLINE.template ../src/ ../src/strife/ > $@

INSTALL.strife: INSTALL.template
	./simplecpp -DSTRIFE -DPRECOMPILED < INSTALL.template > $@


INSTALL: INSTALL.template
	./simplecpp -DDOOM -DHERETIC -DHEXEN -DSTRIFE \
	            -DPRECOMPILED < INSTALL.template > $@

endif

EXTRA_DIST = $(man_MANS) $(MANPAGE_GEN_FILES)        \
             wikipages                               \
             CMDLINE.template                        \
             INSTALL.template                        \
             simplecpp

