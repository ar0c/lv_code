// ui_label_manager.h
#pragma once
#include "lvgl.h"

#define MAX_UI_LABELS 8

typedef struct {
    bool used;
    lv_obj_t *label;
    int x_offset;
    int y_offset;
    uint32_t expire_tick; // 可选：过期自动隐藏
} ui_label_slot_t;

void ui_label_manager_init(void);
void ui_label_show(int id, const char *text, int x_offset, int y_offset, uint32_t duration_ms);
void ui_label_hide(int id);
void ui_label_manager_task(void);  // 定期调用隐藏过期 label
