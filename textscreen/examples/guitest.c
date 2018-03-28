//
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//
// Example program: GUI test program
//
// Demonstrates all the main textscreen widgets in use and shows how
// a simple textscreen program can be written.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "textscreen.h"

enum 
{
    RADIO_VALUE_BADGER,
    RADIO_VALUE_MUSHROOM,
    RADIO_VALUE_SNAKE,
};

// also put some crazy extensions to test the escape function. a"b"c"""dd
const char *extensions[] = { "wad", "lmp", "txt", "a\"b\"c\"\"\"dd", "",
     "\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"", NULL };
const char *radio_values[] = { "Badger", "Mushroom", "Snake" };
char *textbox_value = NULL;
int numbox_value = 0;
int radiobutton_value;
char *file_path = NULL;
char *dir_path = NULL;
txt_label_t *value_label;
txt_window_t *firstwin;
int cheesy;

void ClosePwnBox(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(window))
{
    TXT_CAST_ARG(txt_window_t, window);

    TXT_CloseWindow(window);
}

void PwnBox(TXT_UNCAST_ARG(widget), void *user_data)
{
    txt_window_t *window;
    txt_window_action_t *close_button;
    
    window = TXT_NewWindow("Pwned!");
    TXT_AddWidget(window, TXT_NewLabel(" BOOM! HEADSHOT! "));

    close_button = TXT_NewWindowAction(KEY_ENTER, "Close");
    TXT_SignalConnect(close_button, "pressed", ClosePwnBox, window);

    TXT_SetWindowAction(window, TXT_HORIZ_LEFT, NULL);
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, close_button);
}

void UpdateLabel(TXT_UNCAST_ARG(widget), void *user_data)
{
    char buf[40];
    
    TXT_StringCopy(buf, " Current value: ", sizeof(buf));
    if (cheesy)
    {
        TXT_StringConcat(buf, "Cheesy ", sizeof(buf));
    }
    TXT_StringConcat(buf, radio_values[radiobutton_value], sizeof(buf));
    TXT_StringConcat(buf, "\n", sizeof(buf));

    TXT_SetLabel(value_label, buf);
}

void CloseWindow(TXT_UNCAST_ARG(button), void *user_data)
{
    TXT_CloseWindow(firstwin);
}

void UnicodeWindow(TXT_UNCAST_ARG(widget), void *user_data)
{
    static const char *strings[] = {
        "lunedì", "martedì", "mercoledì", "giovedì",
        "venerdì", "sabato", "domenica",
    };
    static int var1, var2;
    txt_window_t *window;

    window = TXT_NewWindow("Questo è in Italiano");

    TXT_AddWidgets(window,
                   TXT_NewButton("Questo è un tasto"),
                   TXT_NewCheckBox("Questo è un checkbox", &var1),
                   TXT_NewDropdownList(&var2, strings, 7),
                   TXT_NewSeparator("Questo è un separatore"),
                   TXT_NewLabel("Leggi questo, è pieno di\n"
                                "informazioni interessanti"),
                   TXT_NewRadioButton("Ma questo non è un radio??",
                                      &var1, 0),
                   NULL);
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT,
                        TXT_NewWindowAction(KEY_ENTER, "Nullità"));

}

void SetupWindow(void)
{
    txt_window_t *window;
    txt_table_t *table;
    txt_table_t *rightpane;
    txt_checkbox_t *cheesy_checkbox;
    txt_window_action_t *pwn;
    txt_label_t *toplabel;
    char buf[100];
    int i;

    window = TXT_NewWindow("Window test");

    TXT_SetWindowHelpURL(window, "https://www.youtube.com/watch?v=dQw4w9WgXcQ");

    TXT_AddWidget(window, TXT_NewSeparator("Main section"));
    table = TXT_NewTable(3);

    toplabel = TXT_NewLabel("This is a multiline label.\n"
                            "A single label object contains \n"
                            "all three of these lines.");
    TXT_AddWidget(window, toplabel);
    TXT_SetWidgetAlign(toplabel, TXT_HORIZ_CENTER);

    //TXT_AddWidget(window, TXT_NewScrollPane(15, 4, table));
    TXT_AddWidget(window, table);

    for (i=0; i<3; ++i)
    {
        TXT_snprintf(buf, sizeof(buf), "Option %i in a table:", i + 1);
        TXT_AddWidget(table, TXT_NewLabel(buf));
        TXT_snprintf(buf, sizeof(buf), " Button %i-1 ", i + 1);
        TXT_AddWidget(table, TXT_NewButton(buf));
        TXT_snprintf(buf, sizeof(buf), " Button %i-2 ", i + 1);
        TXT_AddWidget(table, TXT_NewButton(buf));
    }

    TXT_AddWidgets(table,
                   TXT_NewLabel("Still the same table, but:\n"
                                "This label magically overflows\n"
                                "across multiple cells! Cool, huh? "),
                   TXT_TABLE_OVERFLOW_RIGHT,
                   TXT_NewButton("Do nothing"),
                   TXT_TABLE_OVERFLOW_DOWN,
                   TXT_TABLE_OVERFLOW_DOWN,
                   TXT_NewButton2("Qualcosa?", UnicodeWindow, NULL),
                   NULL);

    TXT_AddWidget(window, TXT_NewStrut(0, 1));
    value_label = TXT_NewLabel("");
    TXT_AddWidget(window, value_label);

    table = TXT_NewTable(2);
    TXT_AddWidget(window, table);
    TXT_SetWidgetAlign(table, TXT_HORIZ_CENTER);

    cheesy_checkbox = TXT_NewCheckBox("Cheesy", &cheesy);
    TXT_AddWidget(table, cheesy_checkbox);
    TXT_SignalConnect(cheesy_checkbox, "changed", UpdateLabel, NULL);

    rightpane = TXT_NewTable(1);
    TXT_AddWidget(table, rightpane);

    for (i=0; i<3; ++i)
    {
        txt_radiobutton_t *rbut;

        rbut = TXT_NewRadioButton(radio_values[i], &radiobutton_value, i);
        TXT_AddWidget(rightpane, rbut);
        TXT_SignalConnect(rbut, "selected", UpdateLabel, NULL);
    }

    UpdateLabel(NULL, NULL);

    TXT_AddWidget(window, TXT_NewButton2("Close Window", CloseWindow, NULL));

    pwn = TXT_NewWindowAction(KEY_F1, "PWN!");
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, pwn);
    TXT_SignalConnect(pwn, "pressed", PwnBox, NULL);

    firstwin = window;
}

void Window2(void)
{
    txt_window_t *window;
    txt_table_t *table;
    txt_table_t *unselectable_table;
    txt_scrollpane_t *scrollpane;

    window = TXT_NewWindow("Another test");
    TXT_SetWindowPosition(window, 
                          TXT_HORIZ_RIGHT, 
                          TXT_VERT_TOP, 
                          TXT_SCREEN_W - 1, 1);

    TXT_AddWidgets(window,
                   TXT_NewScrollPane(40, 1,
                        TXT_NewLabel("* Unselectable scroll pane *")),
                   unselectable_table = TXT_NewTable(1),
                   NULL);

    TXT_AddWidget(unselectable_table, TXT_NewLabel("* Unselectable table *"));
    TXT_AddWidget(unselectable_table, TXT_NewLabel(
        "This is a UTF-8 string:\n"
        "\xc3\x80 bient\xc3\xb4t na\xc3\xaet "
        "\xc3\xa9v\xc3\xaaque \xc3\xa0 l'\xc5\x93uvre p\xc3\xa8re."));

    TXT_AddWidget(window, TXT_NewSeparator("Input boxes"));
    table = TXT_NewTable(2);
    TXT_AddWidget(window, table);
    TXT_AddWidgets(table,
                   TXT_NewLabel("String: "),
                   TXT_NewInputBox(&textbox_value, 20),
                   TXT_NewLabel("Int: "),
                   TXT_NewIntInputBox(&numbox_value, 10),
                   TXT_NewLabel("Spin control:"),
                   TXT_NewSpinControl(&numbox_value, 0, 15),
                   TXT_NewLabel("File:"),
                   TXT_NewFileSelector(&file_path, 28, "Select file:",
                                       extensions),
                   TXT_NewLabel("Directory:"),
                   TXT_NewFileSelector(&dir_path, 28, "Select directory:",
                                       TXT_DIRECTORY),
                   NULL);

    TXT_AddWidget(window, TXT_NewSeparator("Scroll pane test"));
    scrollpane = TXT_NewScrollPane(40, 5, TXT_NewLabel(
        "This is a scrollable pane. The contents\n"
        "of this box are larger than the box\n"
        "itself, but it can be scrolled around\n"
        "to explore the full contents.\n"
        "\n"
        "Scrollable panes can be scrolled both\n"
        "vertically and horizontally. They\n"
        "can contain any widget. The scroll bars\n"
        "appear automatically as needed.\n"
        "\n"
        "This is a very long line of text that forces a horizontal scrollbar"
    ));
    TXT_AddWidget(window, scrollpane);
}

void ScrollingMenu(void)
{
    txt_window_t *window;
    txt_button_t *button;
    txt_table_t *table;

    window = TXT_NewWindow("Scrollable menu");

    table = TXT_NewTable(1);

    TXT_AddWidgets(table,
                   TXT_NewButton("Configure display"),
                   TXT_NewButton("Configure joystick"),
                   TXT_NewButton("Configure keyboard"),
                   TXT_NewButton("Configure mouse"),
                   TXT_NewButton("Configure sound"),
                   TXT_NewStrut(0, 1),
                   button = TXT_NewButton("Save Parameters and launch DOOM"),
                   TXT_NewStrut(0, 1),
                   TXT_NewButton("Start a network game"),
                   TXT_NewButton("Join a network game"),
                   TXT_NewButton("Multiplayer configuration"),
                   NULL);

    TXT_SignalConnect(button, "pressed", PwnBox, NULL);

    TXT_AddWidget(window, TXT_NewScrollPane(0, 6, table));
}

int main(int argc, char *argv[])
{
    if (!TXT_Init())
    {
        fprintf(stderr, "Failed to initialise GUI\n");
        exit(-1);
    }

    TXT_SetDesktopTitle("Not Chocolate Doom Setup");

    ScrollingMenu();
    Window2();
    SetupWindow();

    TXT_GUIMainLoop();

    TXT_Shutdown();

    return 0;
}


