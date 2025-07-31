#include "ui.h"
#include "lvgl.h"
#include <string.h>
#include "ui_label_manage.h"
#include "freertos/FreeRTOS.h"

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


// 回调函数，用于处理按钮点击事件
static void ok_button_callback(lv_event_t *e) {
    lv_obj_t *btnmatrix = lv_event_get_target(e);
    lv_obj_t *msgbox = lv_obj_get_parent(btnmatrix);  // msgbox
    lv_obj_t *modal = lv_obj_get_parent(msgbox);      // modal container

    lv_obj_del(modal);  // 删除整个 modal，包括 msgbox 和背景
}

// 显示消息框的函数
void show_message_box(const char *title, const char *text) {
    static const char *btns[] = {"OK", "Cancel", NULL}; // 消息框按钮

    // 创建一个消息框
    lv_obj_t *message_box = lv_msgbox_create(NULL, title, text, btns, false);

    // 设置消息框的大小和位置
    lv_obj_set_width(message_box, 300);
    lv_obj_align(message_box, LV_ALIGN_CENTER, 0, 0);

    // 获取消息框中的按钮，并为其添加事件回调
    lv_obj_t *ok_button = lv_msgbox_get_btns(message_box);
    lv_obj_add_event_cb(ok_button, ok_button_callback, LV_EVENT_CLICKED, message_box);

    lv_obj_t *buttons = lv_msgbox_get_btns(message_box);
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
    esp_restart();  // 或 axp2101_shutdown();
    vTaskDelete(NULL);
}
static void shutdown_msgbox_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *msgbox = lv_event_get_target(e);

    if (code == LV_EVENT_VALUE_CHANGED) {
        const char *btn_txt = lv_msgbox_get_active_btn_text(msgbox);
        if (strcmp(btn_txt, "Shutdown") == 0) {
            lv_obj_del(msgbox);
            ui_label_show(2, "Shutting down...", 0, 60, 2000);
            xTaskCreate(shutdown_task, "shutdown_task", 2048, NULL, 5, NULL);
        } else {
            lv_msgbox_close(msgbox);
        }
    }
}

void show_shutdown_prompt(void) {
    lv_obj_t *msgbox = lv_msgbox_create(NULL, "Warning", "Shutdown?", NULL, true);
    lv_obj_center(msgbox);
    lv_obj_t *btn_shutdown = lv_msgbox_add_footer_button(msgbox, "Shutdown");
    lv_obj_add_event_cb(btn_shutdown, shutdown_msgbox_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
}
