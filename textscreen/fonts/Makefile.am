FONT_HDRS = small.h normal.h large.h codepage.h
EXTRA_DIST = small.png normal.png large.png convert-font $(FONT_HDRS)

noinst_DATA = $(FONT_HDRS)

if HAVE_FONTS

small.h: small.png convert-font
	$(srcdir)/convert-font small small.png small.h

normal.h: normal.png convert-font
	$(srcdir)/convert-font normal normal.png normal.h

large.h: large.png convert-font
	$(srcdir)/convert-font large large.png large.h

endif

