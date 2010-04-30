
AM_CFLAGS = -I../src

CTAGS_ARGS=-I TXT_UNCAST_ARG+

# build this directory before the examples directory.

SUBDIRS= . examples

noinst_LIBRARIES=libtextscreen.a

EXTRA_DIST=Doxyfile

libtextscreen_a_SOURCES =                                 \
	textscreen.h                                      \
	txt_checkbox.c           txt_checkbox.h           \
	txt_desktop.c            txt_desktop.h            \
	txt_dropdown.c           txt_dropdown.h           \
	txt_gui.c                txt_gui.h                \
	txt_inputbox.c           txt_inputbox.h           \
	txt_io.c                 txt_io.h                 \
	                         txt_main.h               \
	txt_button.c             txt_button.h             \
	txt_label.c              txt_label.h              \
	txt_radiobutton.c        txt_radiobutton.h        \
	txt_scrollpane.c         txt_scrollpane.h         \
	txt_separator.c          txt_separator.h          \
	txt_spinctrl.c           txt_spinctrl.h           \
	txt_sdl.c                txt_sdl.h                \
	                         txt_smallfont.h          \
	txt_strut.c              txt_strut.h              \
	txt_table.c              txt_table.h              \
	txt_widget.c             txt_widget.h             \
	txt_window.c             txt_window.h             \
	txt_window_action.c      txt_window_action.h      \
	                         txt_font.h

doc:
	doxygen

