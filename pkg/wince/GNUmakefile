
include ../config.make

# Doom:

DOOM_CAB=$(PROGRAM_PREFIX)doom-$(PACKAGE_VERSION).cab
DOOM_CFG=doom-cab.cfg
DOOM_DEPS=$(shell ./wince-cabgen -d $(DOOM_CFG))

# Heretic:

HERETIC_CAB=$(PROGRAM_PREFIX)heretic-$(PACKAGE_VERSION).cab
HERETIC_CFG=heretic-cab.cfg
HERETIC_DEPS=$(shell ./wince-cabgen -d $(HERETIC_CFG))

# Hexen:

HEXEN_CAB=$(PROGRAM_PREFIX)hexen-$(PACKAGE_VERSION).cab
HEXEN_CFG=hexen-cab.cfg
HEXEN_DEPS=$(shell ./wince-cabgen -d $(HEXEN_CFG))

all: $(DOOM_CAB) $(HERETIC_CAB) $(HEXEN_CAB)

$(DOOM_CAB) : $(DOOM_CFG) $(DOOM_DEPS)
	./wince-cabgen $< $@

$(HERETIC_CAB) : $(HERETIC_CFG) $(HERETIC_DEPS)
	./wince-cabgen $< $@

$(HEXEN_CAB) : $(HEXEN_CFG) $(HEXEN_DEPS)
	./wince-cabgen $< $@

clean:
	rm -f $(DOOM_CAB) $(HERETIC_CAB) $(HEXEN_CAB)

