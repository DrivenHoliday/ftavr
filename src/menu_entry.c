#include "menu_entry.h"

#include <assert.h>
#include <string.h>

static void chg_value(menu_entries *entries, uint8_t *value, int8_t change, uint8_t lower, uint8_t upper)
{
    int16_t new = ((int16_t) *value) + change;
    if (new >= lower && new <= upper)
    {
        *value = new;
    }
    else
    {
        (*entries->error_func)();
    }
    menu_entry_display(entries, entries->sseg);
}

static void chg_value_pl(int_payload *payload, int8_t change)
{
    chg_value(payload->entries, payload->value, change, payload->lower, payload->upper);
}

void menu_entry_init(menu_entries *entries, void_func error_func, seven_seg *sseg)
{
    memset(entries, 0, sizeof(*entries));
    entries->error_func = error_func;
    entries->sseg = sseg;
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

void menu_entry_int_value_dec(void *p)
{
    chg_value_pl((int_payload*) p, -1);
}

void menu_entry_int_value_inc(void *p)
{
    chg_value_pl((int_payload*) p, +1);
}

void menu_entry_int_value(void *p, char *ch, uint8_t *dots)
{
    ntostr(ch, *((int_payload*) p)->value);
    (*dots) = ((int_payload*) p)->dots;
}

void menu_entry_add_int(menu_entries *entries, char name[ENTRY_NAME_LEN], uint8_t lower, uint8_t upper, uint8_t dots, uint8_t *value)
{
    int_payload *payload = &(entries->int_payloads[entries->int_payload_num]);
    ++entries->int_payload_num;
    assert(entries->int_payload_num <= MENU_ENTRY_MAX_ENTRIES_NUM);

    payload->lower = lower;
    payload->upper = upper;
    payload->value = value;
    payload->dots = dots;
    payload->entries = entries;
    menu_entry_add(entries, name, menu_entry_int_value, menu_entry_int_value_dec, menu_entry_int_value_inc, payload);
}

void menu_entry_display_entry(menu_entries *entries, seven_seg *sseg, size_t sel)
{
    entries->sel = sel;
    menu_entry_display(entries, sseg);
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
    int8_t selection = ((int8_t) entries->sel) + change;
    while(selection < 0)
    {
        selection += entries->num;
    }
    entries->sel = selection % entries->num;
}

void menu_entry_left_click(menu_entries *entries)
{
    menu_entry entry = entries->menu_entries[entries->sel];
    (*entry.left)(entry.payload);
}

void menu_entry_right_click(menu_entries *entries)
{
    menu_entry entry = entries->menu_entries[entries->sel];
    (*entry.right)(entry.payload);
}

void menu_entry_button_left_click(void *p)
{
    menu_entry_left_click((menu_entries*) p);
}

void menu_entry_button_right_click(void *p)
{
    menu_entry_right_click((menu_entries*) p);
}

void menu_entry_button_left(menu_entries *entries, button_list *butt, mc_port port, mc_pin pin)
{
    button_add(butt, port, pin, menu_entry_button_left_click, entries);
}

void menu_entry_button_right(menu_entries *entries, button_list *butt, mc_port port, mc_pin pin)
{
    button_add(butt, port, pin, menu_entry_button_right_click, entries);
}
