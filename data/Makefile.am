# icons
# 16
icon16dir = $(datadir)/icons/hicolor/16x16/apps
mainicon16 = data/icons/hicolor/16x16/apps/chocolate-doom.png
setupicon16 = data/icons/hicolor/16x16/apps/chocolate-setup.png
# 22
icon22dir = $(datadir)/icons/hicolor/22x22/apps
mainicon22 = data/icons/hicolor/22x22/apps/chocolate-doom.png
setupicon22 = data/icons/hicolor/22x22/apps/chocolate-setup.png
# 24
icon24dir = $(datadir)/icons/hicolor/24x24/apps
mainicon24 = data/icons/hicolor/24x24/apps/chocolate-doom.png
setupicon24 = data/icons/hicolor/24x24/apps/chocolate-setup.png
# 32
icon32dir = $(datadir)/icons/hicolor/32x32/apps
mainicon32 = data/icons/hicolor/32x32/apps/chocolate-doom.png
setupicon32 = data/icons/hicolor/32x32/apps/chocolate-setup.png
# 48
icon48dir = $(datadir)/icons/hicolor/48x48/apps
mainicon48 = data/icons/hicolor/48x48/apps/chocolate-doom.png
setupicon48 = data/icons/hicolor/48x48/apps/chocolate-setup.png
# 256
icon256dir = $(datadir)/icons/hicolor/256x256/apps
mainicon256 = data/icons/hicolor/256x256/apps/chocolate-doom.png
setupicon256 = data/icons/hicolor/256x256/apps/chocolate-setup.png

icon_files = \
    $(mainicon16) \
    $(setupicon16) \
    $(mainicon22) \
    $(setupicon22) \
    $(mainicon24) \
    $(setupicon24) \
    $(mainicon32) \
    $(setupicon32) \
    $(mainicon48) \
    $(setupicon48) \
    $(mainicon256) \
    $(setupicon256) \

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor

install-data-hook: update-icon-cache
uninstall-hook: update-icon-cache
update-icon-cache:
    @-if test -z "$(DESTDIR)"; then \
        echo "Updating Gtk icon cache."; \
        $(gtk_update_icon_cache); \
    else \
        echo "*** Icon cache not updated.  After (un)install, run this:"; \
        echo "***   $(gtk_update_icon_cache)"; \
    fi

EXTRA_DIST=              \
        $(icon_files)    \
        README           \
        doom.ico         \
        doom8.ico        \
        setup.ico        \
        setup8.ico       \
        create-ico