
include ../config.make

TOPLEVEL=../..

EXE_FILES=$(TOPLEVEL)/src/$(PROGRAM_PREFIX)doom.exe    \
          $(TOPLEVEL)/src/$(PROGRAM_PREFIX)server.exe  \
          $(TOPLEVEL)/setup/$(PROGRAM_PREFIX)setup.exe

DLL_FILES=$(TOPLEVEL)/src/SDL.dll                      \
          $(TOPLEVEL)/src/SDL_mixer.dll                \
          $(TOPLEVEL)/src/SDL_net.dll

DOC_FILES += README.OPL

ZIP=$(PACKAGE_TARNAME)-$(PACKAGE_VERSION)-win32.zip

$(ZIP) : staging
	zip -j -r $@ staging/

staging: $(EXE_FILES) $(DLL_FILES) $(patsubst %,../../%,$(DOC_FILES))
	rm -rf staging
	mkdir staging
	cp $(EXE_FILES) $(DLL_FILES) staging/
	$(STRIP) staging/*.exe
	for f in $(DOC_FILES); do                    \
		cp $(TOPLEVEL)/$$f staging/$$f.txt;  \
		unix2dos staging/$$f.txt;            \
	done
	$(TOPLEVEL)/man/simplecpp -D_WIN32 -DPRECOMPILED  \
	         < $(TOPLEVEL)/man/INSTALL.template       \
	         > staging/INSTALL.txt
	unix2dos staging/INSTALL.txt

clean:
	rm -f $(ZIP)
	rm -rf staging

