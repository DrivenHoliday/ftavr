#include "menu_entry.h"

#include <assert.h>
#include <string.h>

void menu_entry_init(menu_entries *entries)
{
    memset(entries, 0, sizeof(entries));
}

void menu_entry_add(menu_entries *entries, char name[ENTRY_NAME_LEN], menu_entry_value_func value, button_func left, button_func right, void *payload)
{
    memcpy(entries->menu_entries[entries->num].name, name, sizeof(entries->menu_entries[entries->num].name));
    entries->menu_entries[entries->num].value = value;
    entries->menu_entries[entries->num].left = left;
    entries->menu_entries[entries->num].right = right;
    entries->menu_entries[entries->num].payload = payload;

    ++entries->num;
    assert(entries->num <= MENU_ENTRY_MAX_ENTRIES_NUM);
}

void menu_entry_display(menu_entries *entries, seven_seg *sseg)
{
    menu_entry entry = entries->menu_entries[entries->sel];
    char text[4];
    char value[2];
    uint8_t dots = 0;

    strncpy(text, entry.name, 2);
    (*entry.value)(entry.payload, value, &dots);
    strncpy(text + 2, value, 2);
    seven_seg_set_chr(sseg, text);
    seven_seg_set_dot(sseg, dots);
}

void menu_entry_chg_select(menu_entries *entries, int8_t change)
{
    //TODO: Maybe problems with negative values?
    entries->sel = (entries->sel + change) % entries->num;
}

void menu_entry_left_click(menu_entries *entries)
{
    menu_entry entry = entries->menu_entries[entries->sel];
    (*entry.left)(entry.payload);
    sseg_display_menu();
}

void menu_entry_right_click(menu_entries *entries)
{
    menu_entry entry = entries->menu_entries[entries->sel];
    (*entry.right)(entry.payload);
    sseg_display_menu();
}
