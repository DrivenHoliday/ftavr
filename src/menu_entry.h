#ifndef MENU_ENTRY_H
#define MENU_ENTRY_H

#include "button.h"
#include "seven_seg.h"

#define ENTRY_NAME_LEN (2)
#define MENU_ENTRY_MAX_ENTRIES_NUM (10)
typedef void(*void_func)(void);
typedef void(*menu_entry_value_func)(void*, char*, uint8_t*);

typedef struct menu_entries_s menu_entries;

struct menu_entry_s
{
    char name[ENTRY_NAME_LEN];
    menu_entry_value_func value;
    button_func left;
    button_func right;
    void *payload;
};

struct int_payload_s {
    uint8_t lower;
    uint8_t upper;
    uint8_t dots;
    uint8_t *value;
    menu_entries *entries;
};

typedef struct menu_entry_s menu_entry;
typedef struct int_payload_s int_payload;

struct menu_entries_s {
    void_func error_func;
    seven_seg *sseg;

    // private!
    menu_entry menu_entries[MENU_ENTRY_MAX_ENTRIES_NUM];
    size_t num;
    size_t sel;

    int_payload int_payloads[MENU_ENTRY_MAX_ENTRIES_NUM];
    size_t int_payload_num;
};

void menu_entry_init(menu_entries *entries, void_func error_func, seven_seg *sseg);
void menu_entry_add(menu_entries *entries, char name[ENTRY_NAME_LEN], menu_entry_value_func value, button_func left, button_func right, void *payload);
void menu_entry_add_int(menu_entries *entries, char name[ENTRY_NAME_LEN], uint8_t lower, uint8_t upper, uint8_t dots, uint8_t *value);
void menu_entry_chg_select(menu_entries *entries, int8_t change);
void menu_entry_left_click(menu_entries *entries);
void menu_entry_right_click(menu_entries *entries);
void menu_entry_display(menu_entries *entries, seven_seg *sseg);
void menu_entry_display_entry(menu_entries *entries, seven_seg *sseg, size_t sel);

void menu_entry_button_left(menu_entries *entries, button *butt, mc_port port, mc_pin pin);
void menu_entry_button_right(menu_entries *entries, button *butt, mc_port port, mc_pin pin);

#endif
