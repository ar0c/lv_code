#include "lvgl.h"
#include "wifi.h"
#include "ui.h"
static void wifi1_btn_cb(lv_event_t *e) {
    show_shutdown_prompt();
}

static void wifi2_btn_cb(lv_event_t *e) {
    show_shutdown_prompt();
}

void create_wifi_buttons_ui(void) {
    static int16_t x = -100;
    static int16_t y = -40;
    // 按钮1：WiFi-1
    lv_obj_t *btn1 = lv_btn_create(lv_scr_act());
    lv_obj_align(btn1, LV_ALIGN_CENTER, x, y);
    lv_obj_add_event_cb(btn1, wifi1_btn_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *label1 = lv_label_create(btn1);
    lv_label_set_text(label1, "Ar0cHome");

    // 按钮2：WiFi-2
    lv_obj_t *btn2 = lv_btn_create(lv_scr_act());
    lv_obj_align(btn2, LV_ALIGN_CENTER, x, y + 80);
    lv_obj_add_event_cb(btn2, wifi2_btn_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *label2 = lv_label_create(btn2);
    lv_label_set_text(label2, "EchoTech");
}
