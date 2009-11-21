
DEPS=$(shell ./wince-cabgen -d $(CONFIG_FILE))
CONFIG_FILE=wince-cab.cfg
OUTPUT_FILE=@PACKAGE_TARNAME@-@PACKAGE_VERSION@.cab

EXTRA_DIST=wince-cabgen $(CONFIG_FILE)

noinst_DATA = $(OUTPUT_FILE)

$(OUTPUT_FILE) : $(CONFIG_FILE) $(DEPS)
	./wince-cabgen $< $@

