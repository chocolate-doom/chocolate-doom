
include ../config.make

TOPLEVEL=../..

DOOM_ZIP=$(PROGRAM_PREFIX)doom-$(PACKAGE_VERSION)-win32.zip
HERETIC_ZIP=$(PROGRAM_PREFIX)heretic-$(PACKAGE_VERSION)-win32.zip
HEXEN_ZIP=$(PROGRAM_PREFIX)hexen-$(PACKAGE_VERSION)-win32.zip
STRIFE_ZIP=$(PROGRAM_PREFIX)strife-$(PACKAGE_VERSION)-win32.zip
ZIPS=$(DOOM_ZIP) $(HERETIC_ZIP) $(HEXEN_ZIP) $(STRIFE_ZIP)

DLL_FILES=$(TOPLEVEL)/src/SDL.dll                      \
          $(TOPLEVEL)/src/SDL_mixer.dll                \
          $(TOPLEVEL)/src/SDL_net.dll

all: $(ZIPS)

$(ZIPS):
	unix2dos $</*.txt
	zip -j -r $@ $</*

$(DOOM_ZIP): staging-doom hook-doom
$(HERETIC_ZIP): staging-heretic
$(HEXEN_ZIP): staging-hexen
$(STRIFE_ZIP): staging-strife hook-strife

# Special hooks to custom modify files for particular games.

hook-doom: staging-doom
	cp $(TOPLEVEL)/NOT-BUGS.md $</NOT-BUGS.txt

# Chocolate Strife has its own custom README file:

hook-strife: staging-strife
	cp $(TOPLEVEL)/README.Strife.md $</README.txt

# Build up a staging dir for a particular game.

staging-%:
	mkdir $@
	./cp-with-libs --ldflags="$(LDFLAGS)" \
	               $(TOPLEVEL)/src/$(PROGRAM_PREFIX)$*.exe $@
	./cp-with-libs --ldflags="$(LDFLAGS)" \
	               $(TOPLEVEL)/src/$(PROGRAM_PREFIX)setup.exe \
	               $@/$(PROGRAM_PREFIX)$*-setup.exe
	./cp-with-libs --ldflags="$(LDFLAGS)" \
	               $(TOPLEVEL)/midiproc/$(PROGRAM_PREFIX)midiproc.exe \
	               $@/$(PROGRAM_PREFIX)midiproc.exe

	$(STRIP) $@/*.exe $@/*.dll

	for f in $(DOC_FILES); do                                \
		cp $(TOPLEVEL)/$$f $@/$$(basename $$f .md).txt;  \
	done
	cp $(TOPLEVEL)/man/CMDLINE.$* $@/CMDLINE.txt

	$(TOPLEVEL)/man/simplecpp -D_WIN32 -DPRECOMPILED  \
	             -D$(shell echo $* | tr a-z A-Z)      \
	         < $(TOPLEVEL)/man/INSTALL.template       \
	         > $@/INSTALL.txt

clean:
	rm -f $(ZIPS)
	rm -rf staging-*
