
include ../config.make

DEPS=$(shell ./wince-cabgen -d $(CONFIG_FILE))
CONFIG_FILE=wince-cab.cfg
OUTPUT_FILE=$(PACKAGE_TARNAME)-$(PACKAGE_VERSION).cab

$(OUTPUT_FILE) : $(CONFIG_FILE) $(DEPS)
	./wince-cabgen $< $@

clean:
	rm -f $(OUTPUT_FILE)

