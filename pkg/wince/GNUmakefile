
include ../config.make

DEPS=$(shell ./wince-cabgen -d $(CONFIG_FILE))
EXECUTABLES=$(filter %.exe, $(DEPS))
CONFIG_FILE=wince-cab.cfg
OUTPUT_FILE=$(PACKAGE_TARNAME)-$(PACKAGE_VERSION).cab

$(OUTPUT_FILE) : $(CONFIG_FILE) $(DEPS)
	$(STRIP) $(EXECUTABLES)
	./wince-cabgen $< $@

clean:
	rm -f $(OUTPUT_FILE)

