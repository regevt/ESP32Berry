/////////////////////////////////////////////////////////////////
/*
  New ESP32Berry Project, The base UI & ChatGPT Client
  For More Information: https://youtu.be/5K6rSw9j5iY
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#pragma once
// #include <lvgl.h>
#include "lvgl.h"
#include <Wire.h>
#include <vector>
#include "Configurations/LGFX_T-Deck.h"
#include "Configurations/ESP32Berry_Config.hpp"

typedef enum
{
    WIFI_RADIO_OFF,
    WIFI_RADIO_ON,
    APP,
    SCREEN_TIMEOUT
} Menu_Event_t;

typedef void (*FuncPtrInt)(Menu_Event_t, void *);

LV_IMG_DECLARE(mouse_cursor_icon);

class Display
{
private:
    TaskHandle_t lvgl_task_handle;
    SemaphoreHandle_t bin_sem;
    friend void update_ui_task(void *pvParameters);
    LGFX *tft;

    lv_indev_t *indev_mouse = nullptr;

    lv_obj_t *ui_Main_Screen;
    lv_obj_t *_bodyScreen;
    lv_obj_t *ui_AppPanel;
    lv_obj_t *ui_AppTitle;
    lv_obj_t *ui_AppCloseBtn;
    lv_obj_t *ui_WiFiLabel;
    lv_obj_t *ui_Userlabel;
    lv_obj_t *ui_TimeLabel;
    lv_obj_t *ui_DateLabel;
    lv_obj_t *ui_BatteryLabel;
    lv_obj_t *ui_Sub_Screen;
    lv_obj_t *ui_Focused_Obj;
    lv_obj_t *ui_Loading;
    lv_obj_t *ui_BasePopup;
    lv_obj_t *ui_BasePopupCloseBtn;
    lv_obj_t *ui_BasePopupTitle;
    lv_obj_t *ui_BasePopupMsg;
    lv_obj_t *ui_LabelScreenTimeoutValue;
    lv_obj_t *ui_ScreenTimeout;

    // Inactivity / screen timeout tracking
    uint32_t last_input_tick = 0;      // FreeRTOS tick of last user interaction
    int screen_timeout_minutes = 5;    // Configured timeout (minutes); 0 means disabled
    bool screen_dimmed = false;        // Whether we've turned the backlight off due to inactivity
    uint8_t previous_brightness = 240; // Brightness to restore after wake

    uint32_t keypad_get_key();
    void initLVGL();
    void ui_main();
    void ui_second();
    void ui_settings();
    void ui_prep_loading();
    void ui_prep_popup_box();
    void ui_popup_open(String title, String msg);
    void HandleKeyboardShortcuts(uint32_t key);
    void inactivity_task_loop(); // Called by RTOS task wrapper
    void fade_backlight_to(uint8_t target, uint8_t step, uint16_t delay_ms);
    static void inactivity_task(void *param);

    String add_battery_icon(int percentage);

public:
    lv_obj_t *ui_NotiLabel;
    lv_obj_t *cursor_obj;
    FuncPtrInt menu_event_cb;
    TaskHandle_t uiNotiTaskHandler;
    lv_obj_t *ui_TopPanel;
    Display(FuncPtrInt callback);
    ~Display();
    void initTFT();
    void ui_WiFi_page();
    void register_activity(); // Mark activity + wake if needed
    void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
    void my_touch_read(lv_indev_t *indev_driver, lv_indev_data_t *data);
    void my_mouse_read(lv_indev_t *indev_driver, lv_indev_data_t *data);
    void my_key_read(lv_indev_t *indev_driver, lv_indev_data_t *data);
    void textarea_event_cb(lv_event_t *e);
    void ui_event_callback(lv_event_t *e);
    void ui_app_btns_callback(lv_event_t *e);
    lv_obj_t *focused_obj();
    void set_focused_obj(lv_obj_t *obj);

    void show_loading_popup(bool isOn);
    void update_time(void *timeStruct);
    void set_notification(const char *msg);
    void update_WiFi_label(void *data1);
    void update_battery(void *data1);
    lv_obj_t *ui_second_screen();
    void goback_main_screen();
    void lv_port_sem_take(void);
    void lv_port_sem_give(void);
    int get_display_width();
    int get_display_height();
    void set_screen_brightness(int value);

    // Public API for screen timeout
    void set_screen_timeout(int minutes); // Update timeout config (minutes)
};

// Global instance pointer used by C thunks to forward LVGL callbacks
extern Display *g_display_instance;
