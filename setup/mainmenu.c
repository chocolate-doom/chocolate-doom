#include <stdlib.h>

#include "config.h"
#include "textscreen.h"

void DoQuit(void *widget, void *dosave)
{
    if (dosave != NULL)
    {
        printf("Saving config\n");
    }

    exit(0);
}

void QuitConfirm(void *unused1, void *unused2)
{
    txt_window_t *window;
    txt_label_t *label;
    txt_button_t *button;

    window = TXT_NewWindow(NULL);

    label = TXT_NewLabel("Save settings and\n"
                         "quit setup?");
    TXT_AddWidget(window, label);
    TXT_AddWidget(window, TXT_NewStrut(24, 0));
    TXT_SetWidgetAlign(label, TXT_HORIZ_CENTER);

    button = TXT_NewButton("  Yes  ");
    TXT_SetWidgetAlign(button, TXT_HORIZ_CENTER);
    TXT_AddWidget(window, button);
    TXT_SignalConnect(button, "pressed", DoQuit, DoQuit);

    button = TXT_NewButton("  No   ");

    // Only an "escape" button in the middle.
    TXT_SetWindowAction(window, TXT_HORIZ_LEFT, NULL);
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, 
                        TXT_NewWindowEscapeAction(window));
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, NULL);

    TXT_SetWidgetAlign(button, TXT_HORIZ_CENTER);
    TXT_AddWidget(window, button);
    TXT_SignalConnect(button, "pressed", DoQuit, NULL);
}

extern void ConfigKeyboard();

void MainMenu(void)
{
    txt_window_t *window;
    txt_window_action_t *quit_action;
    txt_button_t *button;

    window = TXT_NewWindow("Main Menu");
    TXT_AddWidget(window, TXT_NewButton("Configure display"));
    button = TXT_NewButton("Configure keyboard");
    TXT_AddWidget(window, button);
    TXT_SignalConnect(button, "pressed", ConfigKeyboard, NULL);
    TXT_AddWidget(window, TXT_NewButton("Configure mouse"));
    TXT_AddWidget(window, TXT_NewButton("Save parameters and launch DOOM"));
    TXT_AddWidget(window, TXT_NewStrut(0, 1));
    TXT_AddWidget(window, TXT_NewButton("Start a Network game"));
    TXT_AddWidget(window, TXT_NewButton("Join a Network game"));

    quit_action = TXT_NewWindowAction(KEY_ESCAPE, "Abort");
    TXT_SignalConnect(quit_action, "pressed", QuitConfirm, NULL);
    TXT_SetWindowAction(window, TXT_HORIZ_LEFT, quit_action);
}

int main(int argc, char *argv[])
{
    TXT_Init();
    TXT_SetDesktopTitle(PACKAGE_NAME " Setup ver " PACKAGE_VERSION);
    
    MainMenu();

    TXT_GUIMainLoop();
}

