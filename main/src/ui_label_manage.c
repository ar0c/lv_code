#include "lvgl.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "ui_label_manage.h"

static ui_label_slot_t label_slots[MAX_UI_LABELS];

void ui_label_manager_init(void) {
    LV_ASSERT(lv_scr_act());
    memset(label_slots, 0, sizeof(label_slots));
}

void ui_label_show(int id, const char *text, int x_offset, int y_offset, uint32_t duration_ms) {
    if (id < 0 || id >= MAX_UI_LABELS) return;
    ui_label_slot_t *slot = &label_slots[id];

    if (!slot->used) {
        slot->label = lv_label_create(lv_scr_act());
        lv_obj_set_style_text_color(slot->label, lv_color_hex(0x00ff00), LV_PART_MAIN);
        lv_obj_set_style_text_font(slot->label, &lv_font_montserrat_24, LV_PART_MAIN);
        slot->used = true;
    }

    lv_label_set_text(slot->label, text);
    lv_obj_align(slot->label, LV_ALIGN_CENTER, x_offset, y_offset);

    if (duration_ms > 0) {
        slot->expire_tick = lv_tick_get() + duration_ms;
    } else {
        slot->expire_tick = 0; // no auto hide
    }
    lv_obj_clear_flag(slot->label, LV_OBJ_FLAG_HIDDEN);
}

void ui_label_hide(int id) {
    if (id < 0 || id >= MAX_UI_LABELS) return;
    if (!label_slots[id].used) return;

    lv_obj_add_flag(label_slots[id].label, LV_OBJ_FLAG_HIDDEN);
    label_slots[id].expire_tick = 0;
}

void ui_label_manager_task(void) {
    uint32_t now = lv_tick_get();
    for (int i = 0; i < MAX_UI_LABELS; ++i) {
        if (label_slots[i].used && label_slots[i].expire_tick > 0 && now >= label_slots[i].expire_tick) {
            ui_label_hide(i);
        }
    }
}
