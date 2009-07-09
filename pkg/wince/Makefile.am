
# Doom:

DOOM_CAB=chocolate-doom-@PACKAGE_VERSION@.cab
DOOM_CFG=doom-cab.cfg
DOOM_DEPS=$(shell ./wince-cabgen -d $(DOOM_CFG))

$(DOOM_CAB) : $(DOOM_CFG) $(DOOM_DEPS)
	./wince-cabgen $< $@

# Heretic:

HERETIC_CAB=chocolate-heretic-@PACKAGE_VERSION@.cab
HERETIC_CFG=heretic-cab.cfg
HERETIC_DEPS=$(shell ./wince-cabgen -d $(HERETIC_CFG))

$(HERETIC_CAB) : $(HERETIC_CFG) $(HERETIC_DEPS)
	./wince-cabgen $< $@

# Hexen:

HEXEN_CAB=chocolate-hexen-@PACKAGE_VERSION@.cab
HEXEN_CFG=hexen-cab.cfg
HEXEN_DEPS=$(shell ./wince-cabgen -d $(HEXEN_CFG))

$(HEXEN_CAB) : $(HEXEN_CFG) $(HEXEN_DEPS)
	./wince-cabgen $< $@

noinst_DATA = $(DOOM_CAB) $(HERETIC_CAB) $(HEXEN_CAB)

