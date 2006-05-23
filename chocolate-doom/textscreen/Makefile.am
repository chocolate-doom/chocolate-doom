
AM_CFLAGS = @SDL_CFLAGS@ -I../src

noinst_LIBRARIES=libtextscreen.a
bin_PROGRAMS=guitest

libtextscreen_a_SOURCES =                                 \
	txt_checkbox.c           txt_checkbox.h           \
	txt_desktop.c            txt_desktop.h            \
	txt_gui.c                txt_gui.h                \
	txt_inputbox.c           txt_inputbox.h           \
	txt_io.c                 txt_io.h                 \
	txt_main.c               txt_main.h               \
	txt_button.c             txt_button.h             \
	txt_label.c              txt_label.h              \
	txt_radiobutton.c        txt_radiobutton.h        \
	txt_separator.c          txt_separator.h          \
	txt_table.c              txt_table.h              \
	txt_widget.c             txt_widget.h             \
	txt_window.c             txt_window.h             \
	txt_window_action.c      txt_window_action.h      \
	txt_font.h

guitest_LDADD = @SDL_LIBS@ libtextscreen.a
guitest_SOURCES = guitest.c

