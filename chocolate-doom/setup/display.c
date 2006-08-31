#include "textscreen.h"

static int vidmode = 1;
static int fullscreen = 0;
static int grabmouse = 1;

static char *modes[] = { "320x200", "640x400" };

void ConfigDisplay(void)
{
    txt_window_t *window;
    txt_table_t *box;
    
    window = TXT_NewWindow("Display Configuration");

    box = TXT_NewTable(2);
    TXT_AddWidget(box, TXT_NewLabel("Screen mode: "));
    TXT_AddWidget(box, TXT_NewDropdownList(&vidmode, modes, 2));
    TXT_AddWidget(window, box);

    TXT_AddWidget(window, TXT_NewCheckBox("Fullscreen", &fullscreen));
    TXT_AddWidget(window, TXT_NewCheckBox("Grab mouse", &grabmouse));
}

