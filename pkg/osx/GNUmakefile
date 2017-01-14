# Makefile for building the OS X launcher program and DMG package.

include ../config.make

DOC_FILES += README.Strife.md NOT-BUGS.md

export MACOSX_DEPLOYMENT_TARGET=10.7

STAGING_DIR=staging
DMG=$(PACKAGE_TARNAME)-$(PACKAGE_VERSION).dmg

TOPLEVEL=../..
TOPLEVEL_DOCS=$(patsubst %,../../%,$(DOC_FILES))

# DMG file containing package:

$(DMG) : tmp.dmg
	rm -f $@
	./dmgfix "$(realpath tmp.dmg)" "$(PACKAGE_STRING)" "$(PACKAGE_NAME).app"
	hdiutil convert -format UDBZ -o $@ tmp.dmg
	rm -f tmp.dmg

tmp.dmg : $(STAGING_DIR)
	rm -f $@
	hdiutil makehybrid -hfs -hfs-volume-name "$(PACKAGE_STRING)"     \
	                   -hfs-openfolder $(STAGING_DIR) $(STAGING_DIR) \
			   -o tmp.dmg

# Staging dir build for package:

APP_DIR=$(STAGING_DIR)/$(PACKAGE_NAME).app

APP_TOP_DIR=$(APP_DIR)/Contents
APP_BIN_DIR=$(APP_DIR)/Contents/MacOS
SRC_INFO_PLIST=Info.plist

APP_DOC_DIR=$(APP_BIN_DIR)/Documentation
APP_DOC_RELDIR=$(patsubst $(STAGING_DIR)/%,%,$(APP_DOC_DIR))

$(STAGING_DIR): launcher $(TOPLEVEL_DOCS)
	rm -rf $(STAGING_DIR)
	mkdir $(STAGING_DIR)

	mkdir -p "$(APP_TOP_DIR)"
	cp -R Resources "$(APP_TOP_DIR)"
	cp PkgInfo "$(APP_TOP_DIR)"
	cp $(SRC_INFO_PLIST) "$(APP_TOP_DIR)"

	mkdir -p "$(APP_BIN_DIR)"

	mkdir -p "$(APP_DOC_DIR)"
	cp $(TOPLEVEL_DOCS) "$(APP_DOC_DIR)"

	mv "$(APP_DOC_DIR)/README.md" "$(APP_DOC_DIR)/README"
	ln -s "$(APP_DOC_RELDIR)/COPYING" "$(STAGING_DIR)/Software License"
	ln -s "$(APP_DOC_RELDIR)/README" "$(STAGING_DIR)/README"
	ln -s /Applications "$(STAGING_DIR)"

	cp launcher "$(APP_BIN_DIR)"
	$(STRIP) "$(APP_BIN_DIR)/launcher"

	./cp-with-libs $(TOPLEVEL)/src/$(PROGRAM_PREFIX)doom "$(APP_BIN_DIR)"
	$(STRIP) "$(APP_BIN_DIR)/$(PROGRAM_PREFIX)doom"
	./cp-with-libs $(TOPLEVEL)/src/$(PROGRAM_PREFIX)heretic "$(APP_BIN_DIR)"
	$(STRIP) "$(APP_BIN_DIR)/$(PROGRAM_PREFIX)heretic"
	./cp-with-libs $(TOPLEVEL)/src/$(PROGRAM_PREFIX)hexen "$(APP_BIN_DIR)"
	$(STRIP) "$(APP_BIN_DIR)/$(PROGRAM_PREFIX)hexen"
	./cp-with-libs $(TOPLEVEL)/src/$(PROGRAM_PREFIX)strife "$(APP_BIN_DIR)"
	$(STRIP) "$(APP_BIN_DIR)/$(PROGRAM_PREFIX)strife"
	./cp-with-libs $(TOPLEVEL)/src/$(PROGRAM_PREFIX)setup "$(APP_BIN_DIR)"
	$(STRIP) "$(APP_BIN_DIR)/$(PROGRAM_PREFIX)setup"

	$(TOPLEVEL)/man/simplecpp -DPRECOMPILED -D__MACOSX__  \
	                -DDOOM -DHERETIC -DHEXEN -DSTRIFE     \
	             < $(TOPLEVEL)/man/INSTALL.template       \
	             > "$(APP_DOC_DIR)/INSTALL"

	mkdir -p "$(APP_BIN_DIR)/man/man5" "$(APP_BIN_DIR)/man/man6"
	cp $(TOPLEVEL)/man/*.5 "$(APP_BIN_DIR)/man/man5"
	cp $(TOPLEVEL)/man/*.6 "$(APP_BIN_DIR)/man/man6"
	for game in doom heretic hexen strife; do       \
		cp $(TOPLEVEL)/man/CMDLINE.$$game       \
		   "$(APP_DOC_DIR)/CMDLINE-$$game";     \
	done
	cp disk/dir.DS_Store $(STAGING_DIR)/.DS_Store
	cp disk/background.png $(STAGING_DIR)/background.png

clean : launcher_clean
	rm -f $(DMG)
	rm -rf $(STAGING_DIR)

# Launcher build:
CFLAGS = -Wall -I$(TOPLEVEL)
LDFLAGS = -framework Cocoa

LAUNCHER_OBJS= \
        AppController.o \
        Execute.o \
        IWADController.o \
        LauncherManager.o \
        main.o

launcher : $(LAUNCHER_OBJS)
	$(CC) $(LDFLAGS) $(LAUNCHER_OBJS) -o $@

%.o : %.m
	$(CC) -c $(CFLAGS) $^ -o $@

launcher_clean :
	rm -f $(LAUNCHER_OBJS) launcher

