#include "lvgl.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
// 图标字体宏（需保证你定义了 font 图标）
#define NO_WIFI "\uF6AC"
#define WIFI "\uF1EB"

static lv_obj_t *wifi_icon = NULL;
static lv_obj_t *popup_win = NULL;
static lv_obj_t *ta = NULL;
static lv_obj_t *kb = NULL;
static char selected_ssid[64] = {0};

// 示例 WiFi 列表
static const char *wifi_list[] = {
    "HomeWiFi", "ESP32-AP", "MyRouter", "OfficeNet", "Cafe_Free_WiFi"
};

// 声明 UI 弹窗创建函数
static void wifi_ui_create(lv_event_t *e);

/**
 * @brief 更新 WiFi 图标状态（连接/未连接）
 */
static void wifi_icon_status_update(bool is_connected) {
    if (wifi_icon == NULL) {
        wifi_icon = lv_label_create(lv_scr_act());
        lv_obj_align(wifi_icon, LV_ALIGN_TOP_LEFT, 10, 10);
    }
    lv_obj_add_flag(wifi_icon, LV_OBJ_FLAG_CLICKABLE);              // 可点击
    lv_obj_add_event_cb(wifi_icon, wifi_ui_create, LV_EVENT_CLICKED, NULL); // 初次绑定点击事件

    if (is_connected) {
        lv_label_set_text(wifi_icon, WIFI);
    } else {
        lv_label_set_text(wifi_icon, NO_WIFI);
    }

    // 设置图标字体（假设 icon_all 是你定义的字体）
    extern const lv_font_t icon_all;
    lv_obj_set_style_text_font(wifi_icon, &icon_all, LV_PART_MAIN);
}

/**
 * @brief 关闭弹窗窗口
 */
static void close_popup(lv_event_t *e) {
    if (popup_win) {
        lv_obj_del(popup_win);
        popup_win = NULL;
    }
}

/**
 * @brief 模拟连接 WiFi
 */
static void connect_to_wifi(const char *ssid, const char *password) {
    printf("Connecting to SSID: %s, Password: %s\n", ssid, password);
    wifi_icon_status_update(true);
    close_popup(NULL);
}

/**
 * @brief 键盘事件处理（确认连接 / 取消）
 */
static void on_keyboard_event(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_READY) {
        const char *pwd = lv_textarea_get_text(ta);
        connect_to_wifi(selected_ssid, pwd);
    }

    if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        if (kb) lv_obj_del(kb);
        if (ta) lv_obj_del(ta);
        kb = NULL;
        ta = NULL;
    }
}

/**
 * @brief 弹出密码输入框
 */
static void show_password_input(const char *ssid) {
    strncpy(selected_ssid, ssid, sizeof(selected_ssid));
    selected_ssid[sizeof(selected_ssid) - 1] = '\0';

    ta = lv_textarea_create(lv_scr_act());
    lv_obj_set_width(ta, 200);
    lv_obj_center(ta);
    lv_textarea_set_password_mode(ta, true);
    lv_textarea_set_placeholder_text(ta, "Enter Password");

    kb = lv_keyboard_create(lv_scr_act());
    lv_keyboard_set_textarea(kb, ta);
    lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_TEXT_LOWER);
    lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_event_cb(kb, on_keyboard_event, LV_EVENT_ALL, NULL);
}

/**
 * @brief WiFi 列表项点击事件
 */
static void on_wifi_item_clicked(lv_event_t *e) {
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *list = lv_obj_get_parent(btn);
    const char *ssid = lv_list_get_button_text(list, btn);
    show_password_input(ssid);
}

/**
 * @brief 创建 WiFi 弹出窗口（列表 + 关闭按钮）
 */
 static void wifi_ui_create(lv_event_t *e) {
    if (popup_win) return;

    // 创建窗口
    popup_win = lv_win_create(lv_scr_act());
    lv_obj_set_size(popup_win, 280, 300);
    lv_obj_center(popup_win);

    // 添加标题栏
    lv_obj_t *title = lv_win_add_title(popup_win, "Choose WiFi");

    // 添加关闭按钮
    lv_obj_t *close_btn = lv_btn_create(title);
    lv_obj_set_size(close_btn, 30, 30);
    lv_obj_align(close_btn, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_add_event_cb(close_btn, close_popup, LV_EVENT_CLICKED, NULL);
    lv_obj_t *close_label = lv_label_create(close_btn);
    lv_label_set_text(close_label, LV_SYMBOL_CLOSE);

    // 创建 WiFi 列表：直接添加到 window 中（不需要 set_content）
    lv_obj_t *list = lv_list_create(popup_win);
    lv_obj_set_size(list, lv_obj_get_width(popup_win) - 20, lv_obj_get_height(popup_win) - 60);
    lv_obj_align(list, LV_ALIGN_BOTTOM_MID, 0, 0);  // 手动定位内容在下方

    // 添加按钮项
    for (int i = 0; i < sizeof(wifi_list) / sizeof(wifi_list[0]); ++i) {
        lv_obj_t *btn = lv_list_add_button(list, NULL, wifi_list[i]);
        // 添加对号图标
        if (selected_ssid && strcmp(wifi_list[i], selected_ssid) == 0) {
            lv_obj_t *label = lv_label_create(btn);
            lv_label_set_text(label, LV_SYMBOL_OK);
            lv_obj_align(label, LV_ALIGN_RIGHT_MID, -5, 0);  // 靠右显示
        }
        lv_obj_add_event_cb(btn, on_wifi_item_clicked, LV_EVENT_CLICKED, NULL);
    }

}


/**
 * @brief 启动函数，创建图标并显示未连接状态
 */
void create_wifi_ui(void) {
    wifi_icon_status_update(false);  // 默认未连接
}
