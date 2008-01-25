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
char *radio_values[] = { "Badger", "Mushroom", "Snake" };
char *textbox_value = NULL;
int numbox_value = 0;
int radiobutton_value;
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
    
    strcpy(buf, " Current value: ");
    if (cheesy)
    {
        strcat(buf, "Cheesy ");
    }
    strcat(buf, radio_values[radiobutton_value]);
    strcat(buf, "\n");

    TXT_SetLabel(value_label, buf);
}

void CloseWindow(TXT_UNCAST_ARG(button), void *user_data)
{
    TXT_CloseWindow(firstwin);
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

    TXT_AddWidget(window, TXT_NewSeparator("Main section"));
    table = TXT_NewTable(3);

    toplabel = TXT_NewLabel("This is a multiline label.\n"
                            "A single label object contains \n"
                            "all three of these lines.\n");
    TXT_AddWidget(window, toplabel);
    TXT_SetWidgetAlign(toplabel, TXT_HORIZ_CENTER);

    TXT_AddWidget(window, table);

    for (i=0; i<5; ++i)
    {
        sprintf(buf, "Option %i in a table:", i + 1);
        TXT_AddWidget(table, TXT_NewLabel(buf));
        sprintf(buf, " Button %i-1 ", i + 1);
        TXT_AddWidget(table, TXT_NewButton(buf));
        sprintf(buf, " Button %i-2 ", i + 1);
        TXT_AddWidget(table, TXT_NewButton(buf));
    }

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
    int i;
    
    window = TXT_NewWindow("Another test");
    TXT_SetWindowPosition(window, 
                          TXT_HORIZ_RIGHT, 
                          TXT_VERT_TOP, 
                          TXT_SCREEN_W - 1, 1);

    for (i=0; i<5; ++i)
    {
        TXT_AddWidget(window, TXT_NewButton("hello there blah blah blah blah"));
    }

    TXT_AddWidget(window, TXT_NewSeparator("Input boxes"));
    table = TXT_NewTable(2);
    TXT_AddWidget(window, table);
    TXT_AddWidget(table, TXT_NewLabel("String: "));
    TXT_AddWidget(table, TXT_NewInputBox(&textbox_value, 30));
    TXT_AddWidget(table, TXT_NewLabel("Int: "));
    TXT_AddWidget(table, TXT_NewIntInputBox(&numbox_value, 10));
    TXT_AddWidget(table, TXT_NewLabel("Spin control:"));
    TXT_AddWidget(table, TXT_NewSpinControl(&numbox_value, 0, 15));
}

int main(int argc, char *argv[])
{
    if (!TXT_Init())
    {
        fprintf(stderr, "Failed to initialise GUI\n");
        exit(-1);
    }

    TXT_SetDesktopTitle("Not Chocolate Doom Setup");

    Window2();
    SetupWindow();

    TXT_GUIMainLoop();

    TXT_Shutdown();

    return 0;
}


