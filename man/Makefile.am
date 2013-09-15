
MANPAGE_GEN_FILES=\
        doom.template \
        heretic.template \
        hexen.template \
        strife.template \
        docgen \
        default.cfg.template \
        extra.cfg.template

docdir=$(prefix)/share/doc/@PACKAGE@

if HAVE_PYTHON

man_MANS=chocolate-server.6       \
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

nodist_doc_DATA=INSTALL CMDLINE.doom CMDLINE.heretic \
                CMDLINE.hexen CMDLINE.strife



chocolate-doom.6: ../src $(MANPAGE_GEN_FILES)
	./docgen -g doom -m doom.template ../src ../src/doom > $@

default.cfg.5: ../src default.cfg.template
	./docgen -g doom -m default.cfg.template \
                 -c default ../src/m_config.c > $@

chocolate-doom.cfg.5: ../src extra.cfg.template
	./docgen -g doom -m extra.cfg.template \
                 -c extended ../src/m_config.c > $@

CMDLINE.doom : ../src ../src/doom
	./docgen -p CMDLINE.template ../src/ ../src/doom/ > $@



chocolate-heretic.6: ../src $(MANPAGE_GEN_FILES)
	./docgen -g heretic -m heretic.template ../src ../src/heretic > $@

heretic.cfg.5: ../src default.cfg.template
	./docgen -g heretic -m default.cfg.template \
                 -c default ../src/m_config.c > $@

chocolate-heretic.cfg.5: ../src extra.cfg.template
	./docgen -g heretic -m extra.cfg.template \
                 -c extended ../src/m_config.c > $@

CMDLINE.heretic : ../src ../src/heretic
	./docgen -p CMDLINE.template ../src/ ../src/heretic/ > $@


chocolate-hexen.6: ../src $(MANPAGE_GEN_FILES)
	./docgen -g hexen -m hexen.template ../src ../src/hexen > $@

hexen.cfg.5: ../src default.cfg.template
	./docgen -g hexen -m default.cfg.template \
                 -c default ../src/m_config.c > $@

chocolate-hexen.cfg.5: ../src extra.cfg.template
	./docgen -g hexen -m extra.cfg.template \
                 -c extended ../src/m_config.c > $@

CMDLINE.hexen : ../src ../src/hexen
	./docgen -p CMDLINE.template ../src/ ../src/hexen/ > $@



chocolate-strife.6: ../src $(MANPAGE_GEN_FILES)
	./docgen -g strife -m strife.template ../src ../src/strife > $@

strife.cfg.5: ../src default.cfg.template
	./docgen -g strife -m default.cfg.template \
                 -c default ../src/m_config.c > $@

chocolate-strife.cfg.5: ../src extra.cfg.template
	./docgen -g strife -m extra.cfg.template \
                 -c extended ../src/m_config.c > $@

CMDLINE.strife : ../src ../src/strife
	./docgen -p CMDLINE.template ../src/ ../src/strife/ > $@



INSTALL: INSTALL.template
	./simplecpp -DPRECOMPILED < INSTALL.template > $@

endif

EXTRA_DIST = $(man_MANS) $(MANPAGE_GEN_FILES)        \
             wikipages                               \
             CMDLINE.template                        \
             INSTALL.template                        \
             simplecpp

