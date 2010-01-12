
include ../config.make

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
APP_BIN_DIR=$(APP_DIR)/Contents/MacOS/

$(STAGING_DIR): launcher $(TOPLEVEL_DOCS)
	rm -rf $(STAGING_DIR)
	mkdir $(STAGING_DIR)

	cp -R app-skeleton "$(APP_DIR)"
	mkdir "$(APP_BIN_DIR)"
	cp Info.plist "$(APP_DIR)/Contents/"

	cp launcher "$(APP_BIN_DIR)/"
	$(STRIP) "$(APP_BIN_DIR)/launcher"

	./cp-with-libs $(TOPLEVEL)/src/$(PACKAGE_TARNAME) "$(APP_BIN_DIR)"
	$(STRIP) "$(APP_BIN_DIR)/$(PACKAGE_TARNAME)"
	./cp-with-libs $(TOPLEVEL)/setup/chocolate-setup "$(APP_BIN_DIR)"
	$(STRIP) "$(APP_BIN_DIR)/chocolate-setup"

	cp $(TOPLEVEL_DOCS) "$(STAGING_DIR)"

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

