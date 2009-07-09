
DEPS=$(shell ./wince-cabgen -d $(CONFIG_FILE))
CONFIG_FILE=wince-cab.cfg
OUTPUT_FILE=@PACKAGE_TARNAME@-@PACKAGE_VERSION@.cab

noinst_DATA = $(OUTPUT_FILE)

$(OUTPUT_FILE) : $(CONFIG_FILE) $(DEPS)
	./wince-cabgen $< $@

