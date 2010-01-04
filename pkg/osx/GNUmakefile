
include config.make

STAGING_DIR=staging
DMG=$(PACKAGE_TARNAME)-$(PACKAGE_VERSION).dmg

# DMG file containing package:

$(DMG) : $(STAGING_DIR)
	rm -f $@
	hdiutil create -volname "$(PACKAGE_STRING)" -srcdir $(STAGING_DIR) $@

clean : launcher_clean
	rm -f $(DMG)
	rm -rf $(STAGING_DIR)

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
	find $(STAGING_DIR) -name .svn -delete -exec rm -rf {} \;

# Launcher build:

CFLAGS=-Wall
LDFLAGS=-framework Cocoa

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

