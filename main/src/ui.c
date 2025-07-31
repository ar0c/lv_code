#include "ui.h"
#include "lvgl.h"
#include <string.h>
#include "ui_label_manage.h"
#include <stdlib.h>   // for malloc, free
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"

lv_obj_t *create_colored_label(const char *text,int x_offset, int y_offset)
{
    lv_color_t color = lv_color_hex(0x00ff00);
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, color, LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, x_offset, y_offset);
    return label;
}
void create_or_update_label_once(lv_obj_t **label_ref, const char *text, int x_offset, int y_offset)
{
    if (!*label_ref) {
        *label_ref = lv_label_create(lv_scr_act());
        lv_color_t color = lv_color_hex(0x00ff00);
        lv_obj_set_style_text_color(*label_ref, color, LV_PART_MAIN);
        lv_obj_set_style_text_font(*label_ref, &lv_font_montserrat_24, LV_PART_MAIN);
        lv_obj_align(*label_ref, LV_ALIGN_CENTER, x_offset, y_offset);
    } else {
        lv_obj_align(*label_ref, LV_ALIGN_CENTER, x_offset, y_offset); // 可选位置更新
    }

    lv_label_set_text(*label_ref, text);
}


typedef struct {
    char text[64];
    int x_offset;
    int y_offset;
} ui_text_param_t;

static void ui_show_text_cb(void *param)
{
    ui_text_param_t *p = (ui_text_param_t *)param;
    create_colored_label(p->text, p->x_offset, p->y_offset);
    free(p);  // 使用 malloc 申请的内存需要释放
}
void ui_show_text_async(const char *text, int x_offset, int y_offset)
{
    ui_text_param_t *p = malloc(sizeof(ui_text_param_t));
    if (!p) return;

    snprintf(p->text, sizeof(p->text), "%s", text);
    p->x_offset = x_offset;
    p->y_offset = y_offset;

    lv_async_call(ui_show_text_cb, p);
}

static lv_obj_t *label_wifi_status = NULL;

void wifi_status_ui_init(void)
{
    label_wifi_status = lv_label_create(lv_scr_act());
    lv_obj_align(label_wifi_status, LV_ALIGN_TOP_RIGHT, -20, 10);
    lv_obj_set_style_text_color(label_wifi_status, lv_color_hex(0x0000ff), LV_PART_MAIN);
    lv_label_set_text(label_wifi_status, "WiFi: Idle");
}

void wifi_status_ui_set_text(const char *text)
{
    if (label_wifi_status) {
        lv_label_set_text(label_wifi_status, text);
    }
}

void ui_init(void)
{
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0xffffff), LV_PART_MAIN);
}
static void shutdown_task(void *arg) {
    vTaskDelay(pdMS_TO_TICKS(500));  // 可选延迟
    vTaskDelete(NULL);
}
static void shutdown_msgbox_event_cb(lv_event_t *e) {
    lv_obj_t * msgbox = lv_event_get_user_data(e);
    lv_msgbox_close(msgbox);
    ui_label_show(2, "Shutting down...", 0, 60, 2000);
    xTaskCreate(shutdown_task, "shutdown_task", 2048, NULL, 5, NULL);
}

void show_shutdown_prompt(void) {
    lv_obj_t *msgbox = lv_msgbox_create(NULL);
    lv_msgbox_add_title(msgbox, "Shutdown?");
    lv_msgbox_add_text(msgbox, "Are you sure you want to shutdown?");
    lv_obj_t * shutdownBtn = lv_msgbox_add_footer_button(msgbox, "Shutdown");
    lv_obj_t * cancelBtn = lv_msgbox_add_footer_button(msgbox, "Cancel");
    lv_obj_add_event_cb(shutdownBtn, shutdown_msgbox_event_cb, LV_EVENT_CLICKED, msgbox);
    lv_obj_add_event_cb(cancelBtn, shutdown_msgbox_event_cb, LV_EVENT_CLICKED, msgbox);
    lv_obj_center(msgbox);
}
