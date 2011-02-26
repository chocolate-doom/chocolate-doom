
# Makefile for building the OS X launcher program and DMG package.
# It is also possible to build and run the launcher under Unix
# systems using GNUstep, although this is only here for development
# and debugging purposes.

include ../config.make

# Build so that the package will work on older versions.

export MACOSX_DEPLOYMENT_TARGET=10.4

STAGING_DIR=staging
DMG=$(PACKAGE_TARNAME)-$(PACKAGE_VERSION).dmg

TOPLEVEL=../..
TOPLEVEL_DOCS=$(patsubst %,../../%,$(DOC_FILES))

ifndef GNUSTEP_MAKEFILES

# DMG file containing package:

$(DMG) : $(STAGING_DIR)
	rm -f $@
	hdiutil create -volname "$(PACKAGE_STRING)" -srcdir $(STAGING_DIR) $@

endif

# Staging dir build for package:

APP_DIR=$(STAGING_DIR)/$(PACKAGE_NAME).app

# OS X and GNUstep apps have a slightly different internal structure:
# OS X apps have their files within a containing "Contents" directory
# that does not exist in GNUstep apps.  Similarly, the binaries are
# installed at the top level, rather than in a "MacOS" directory.
# Finally, we must install a different Info.plist file.

ifdef GNUSTEP_MAKEFILES
APP_TOP_DIR=$(APP_DIR)
APP_BIN_DIR=$(APP_DIR)
SRC_INFO_PLIST=Info-gnustep.plist
else
APP_TOP_DIR=$(APP_DIR)/Contents
APP_BIN_DIR=$(APP_DIR)/Contents/MacOS
SRC_INFO_PLIST=Info.plist
endif

$(STAGING_DIR): launcher $(TOPLEVEL_DOCS)
	rm -rf $(STAGING_DIR)
	mkdir $(STAGING_DIR)

	cp $(TOPLEVEL_DOCS) "$(STAGING_DIR)"

	mkdir -p "$(APP_TOP_DIR)"
	cp -R Resources "$(APP_TOP_DIR)"
	cp PkgInfo "$(APP_TOP_DIR)"
	cp $(SRC_INFO_PLIST) "$(APP_TOP_DIR)"

	mkdir -p "$(APP_BIN_DIR)"

	cp launcher "$(APP_BIN_DIR)"
	$(STRIP) "$(APP_BIN_DIR)/launcher"

	./cp-with-libs $(TOPLEVEL)/src/$(PACKAGE_TARNAME) "$(APP_BIN_DIR)"
	$(STRIP) "$(APP_BIN_DIR)/$(PACKAGE_TARNAME)"
	./cp-with-libs $(TOPLEVEL)/setup/chocolate-setup "$(APP_BIN_DIR)"
	$(STRIP) "$(APP_BIN_DIR)/chocolate-setup"

	find $(STAGING_DIR) -name .svn -delete -exec rm -rf {} \; || true

clean : launcher_clean
	rm -f $(DMG)
	rm -rf $(STAGING_DIR)

# Launcher build:

CFLAGS = -Wall -I$(TOPLEVEL)

# Are we building using gs_make?

ifdef GNUSTEP_MAKEFILES
CFLAGS += $(shell gnustep-config --objc-flags)
LDFLAGS = $(shell gnustep-config --gui-libs)
else
LDFLAGS = -framework Cocoa
endif

LAUNCHER_OBJS= \
        AppController.o \
        Execute.o \
        IWADController.o \
        IWADLocation.o \
        LauncherManager.o \
        main.o

launcher : $(LAUNCHER_OBJS)
	$(CC) $(LDFLAGS) $(LAUNCHER_OBJS) -o $@

%.o : %.m
	$(CC) -c $(CFLAGS) $^ -o $@

launcher_clean :
	rm -f $(LAUNCHER_OBJS) launcher

