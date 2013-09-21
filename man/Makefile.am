
MANPAGE_GEN_FILES=\
        doom.template \
        heretic.template \
        hexen.template \
        strife.template \
        docgen \
        default.cfg.template \
        extra.cfg.template

doomdocsdir = $(prefix)/share/doc/@PROGRAM_PREFIX@doom
hereticdocsdir = $(prefix)/share/doc/@PROGRAM_PREFIX@heretic
hexendocsdir = $(prefix)/share/doc/@PROGRAM_PREFIX@hexen
strifedocsdir = $(prefix)/share/doc/@PROGRAM_PREFIX@strife

if HAVE_PYTHON

man_MANS = chocolate-server.6       \
           chocolate-setup.6        \
           chocolate-doom.6         \
           default.cfg.5            \
           chocolate-doom.cfg.5     \
           chocolate-heretic.6      \
           heretic.cfg.5            \
           chocolate-heretic.cfg.5  \
           chocolate-hexen.6        \
           hexen.cfg.5              \
           chocolate-hexen.cfg.5    \
           chocolate-strife.6       \
           strife.cfg.5             \
           chocolate-strife.cfg.5

doomdocs_DATA    = INSTALL.doom    CMDLINE.doom
hereticdocs_DATA = INSTALL.heretic CMDLINE.heretic
hexendocs_DATA   = INSTALL.hexen   CMDLINE.hexen
strifedocs_DATA  = INSTALL.strife  CMDLINE.strife

chocolate-doom.6: ../src $(MANPAGE_GEN_FILES)
	./docgen -g doom -m doom.template ../src ../src/doom > $@

default.cfg.5: ../src default.cfg.template
	./docgen -g doom -m default.cfg.template \
                 -c default ../src/m_config.c > $@

chocolate-doom.cfg.5: ../src extra.cfg.template
	./docgen -g doom -m extra.cfg.template \
                 -c extended ../src/m_config.c > $@

CMDLINE.doom : CMDLINE.template ../src ../src/doom
	./docgen -p CMDLINE.template ../src/ ../src/doom/ > $@

INSTALL.doom: INSTALL.template
	./simplecpp -DDOOM -DPRECOMPILED < INSTALL.template > $@


chocolate-heretic.6: ../src $(MANPAGE_GEN_FILES)
	./docgen -g heretic -m heretic.template ../src ../src/heretic > $@

heretic.cfg.5: ../src default.cfg.template
	./docgen -g heretic -m default.cfg.template \
                 -c default ../src/m_config.c > $@

chocolate-heretic.cfg.5: ../src extra.cfg.template
	./docgen -g heretic -m extra.cfg.template \
                 -c extended ../src/m_config.c > $@

CMDLINE.heretic : CMDLINE.template ../src ../src/heretic
	./docgen -p CMDLINE.template ../src/ ../src/heretic/ > $@

INSTALL.heretic: INSTALL.template
	./simplecpp -DHERETIC -DPRECOMPILED < INSTALL.template > $@


chocolate-hexen.6: ../src $(MANPAGE_GEN_FILES)
	./docgen -g hexen -m hexen.template ../src ../src/hexen > $@

hexen.cfg.5: ../src default.cfg.template
	./docgen -g hexen -m default.cfg.template \
                 -c default ../src/m_config.c > $@

chocolate-hexen.cfg.5: ../src extra.cfg.template
	./docgen -g hexen -m extra.cfg.template \
                 -c extended ../src/m_config.c > $@

CMDLINE.hexen : CMDLINE.template ../src ../src/hexen
	./docgen -p CMDLINE.template ../src/ ../src/hexen/ > $@

INSTALL.hexen: INSTALL.template
	./simplecpp -DHEXEN -DPRECOMPILED < INSTALL.template > $@


chocolate-strife.6: ../src $(MANPAGE_GEN_FILES)
	./docgen -g strife -m strife.template ../src ../src/strife > $@

strife.cfg.5: ../src default.cfg.template
	./docgen -g strife -m default.cfg.template \
                 -c default ../src/m_config.c > $@

chocolate-strife.cfg.5: ../src extra.cfg.template
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

