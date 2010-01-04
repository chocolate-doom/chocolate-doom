
include config.make

STAGING_DIR=staging
DMG=$(PACKAGE_TARNAME)-$(PACKAGE_VERSION).dmg

ifndef GNUSTEP_MAKEFILES

# DMG file containing package:

$(DMG) : $(STAGING_DIR)
	rm -f $@
	hdiutil create -volname "$(PACKAGE_STRING)" -srcdir $(STAGING_DIR) $@

endif

# Staging dir build for package:

APP_DIR=$(STAGING_DIR)/$(PACKAGE_NAME).app

$(STAGING_DIR): launcher
	rm -rf $(STAGING_DIR)
	mkdir $(STAGING_DIR)
	cp -R app-skeleton "$(APP_DIR)"
	cp Info.plist "$(APP_DIR)/Contents/"
	cp launcher "$(APP_DIR)/Contents/MacOS/"
	# TODO: copy Doom and setup binaries into app dir
	# TODO: copy other documentation into staging dir
	find $(STAGING_DIR) -name .svn -delete -exec rm -rf {} \; || true

clean : launcher_clean
	rm -f $(DMG)
	rm -rf $(STAGING_DIR)

# Launcher build:

CFLAGS = -Wall -I../..

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

