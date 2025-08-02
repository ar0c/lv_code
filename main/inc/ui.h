#ifndef UI_H
#define UI_H

void ui_init(void);
void ui_show_text_async(const char *text, int x_offset, int y_offset);
void show_message_box(const char *title, const char *text);
void ui_show_text_cb(void *param);
void show_shutdown_prompt(void);

#endif // UI_H
