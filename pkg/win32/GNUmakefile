
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
	zip -j -r $@ $</*

$(DOOM_ZIP): staging-doom hook-doom
$(HERETIC_ZIP): staging-heretic
$(HEXEN_ZIP): staging-hexen
$(STRIFE_ZIP): staging-strife hook-strife

# Special hooks to custom modify files for particular games.

hook-doom: staging-doom
	-pandoc -f gfm -s -o $</NOT-BUGS.html $(TOPLEVEL)/NOT-BUGS.md

# Chocolate Strife has its own custom README file:

hook-strife: staging-strife
	-pandoc -f gfm -s -o $</README.html $(TOPLEVEL)/README.Strife.md

# Build up a staging dir for a particular game.

staging-%:
	mkdir $@
	LC_ALL=C ./cp-with-libs --ldflags="$(LDFLAGS)" \
	               $(TOPLEVEL)/src/$(PROGRAM_PREFIX)$*.exe $@
	LC_ALL=C ./cp-with-libs --ldflags="$(LDFLAGS)" \
	               $(TOPLEVEL)/src/$(PROGRAM_PREFIX)setup.exe \
	               $@/$(PROGRAM_PREFIX)$*-setup.exe
	LC_ALL=C ./cp-with-libs --ldflags="$(LDFLAGS)" \
	               $(TOPLEVEL)/midiproc/$(PROGRAM_PREFIX)midiproc.exe \
	               $@/$(PROGRAM_PREFIX)midiproc.exe

	$(STRIP) $@/*.exe $@/*.dll

	-for f in $(DOC_FILES); do                              \
		pandoc  -f gfm -o $@/$$(basename $$f .md).html  \
			-s $(TOPLEVEL)/$$f;                     \
	done
	-pandoc -f gfm -s -o $@/CMDLINE.html $(TOPLEVEL)/man/CMDLINE.$*.md
	-pandoc -f gfm -s -o $@/INSTALL.html $(TOPLEVEL)/man/INSTALL.$*

clean:
	rm -f $(ZIPS)
	rm -rf staging-*
