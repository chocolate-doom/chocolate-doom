
MANPAGE_GEN_FILES=manpage.template docgen default.cfg.template extra.cfg.template

docdir=$(prefix)/share/doc/@PACKAGE@

if HAVE_PYTHON

man_MANS=cnserver.6       \
         cnsetup.6        \
         cndoom.6         \
         default.cfg.5            \
         cndoom.cfg.5     \
         cnheretic.6      \
         heretic.cfg.5            \
         cnheretic.cfg.5  \
         cnhexen.6        \
         hexen.cfg.5              \
         cnhexen.cfg.5    \
         cnstrife.6       \
         strife.cfg.5             \
         cnstrife.cfg.5

nodist_doc_DATA=INSTALL



cndoom.6: ../src $(MANPAGE_GEN_FILES)
	./docgen -g doom -m manpage.template ../src ../src/doom > $@

default.cfg.5: ../src default.cfg.template
	./docgen -g doom -m default.cfg.template \
                 -c default ../src/m_config.c > $@

cndoom.cfg.5: ../src extra.cfg.template
	./docgen -g doom -m extra.cfg.template \
                 -c extended ../src/m_config.c > $@



cnheretic.6: ../src $(MANPAGE_GEN_FILES)
	./docgen -g heretic -m manpage.template ../src ../src/heretic > $@

heretic.cfg.5: ../src default.cfg.template
	./docgen -g heretic -m default.cfg.template \
                 -c default ../src/m_config.c > $@

cnheretic.cfg.5: ../src extra.cfg.template
	./docgen -g heretic -m extra.cfg.template \
                 -c extended ../src/m_config.c > $@



cnhexen.6: ../src $(MANPAGE_GEN_FILES)
	./docgen -g hexen -m manpage.template ../src ../src/hexen > $@

hexen.cfg.5: ../src default.cfg.template
	./docgen -g hexen -m default.cfg.template \
                 -c default ../src/m_config.c > $@

cnhexen.cfg.5: ../src extra.cfg.template
	./docgen -g hexen -m extra.cfg.template \
                 -c extended ../src/m_config.c > $@



cnstrife.6: ../src $(MANPAGE_GEN_FILES)
	./docgen -g strife -m manpage.template ../src ../src/strife > $@

strife.cfg.5: ../src default.cfg.template
	./docgen -g strife -m default.cfg.template \
                 -c default ../src/m_config.c > $@

cnstrife.cfg.5: ../src extra.cfg.template
	./docgen -g strife -m extra.cfg.template \
                 -c extended ../src/m_config.c > $@



INSTALL: INSTALL.template
	./simplecpp -DPRECOMPILED < INSTALL.template > $@

endif

EXTRA_DIST = $(man_MANS) $(MANPAGE_GEN_FILES)        \
             wikipages                               \
             CMDLINE.template                        \
             INSTALL.template                        \
             simplecpp

