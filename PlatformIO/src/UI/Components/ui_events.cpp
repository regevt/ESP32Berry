/////////////////////////////////////////////////////////////////
/*
  LVGL event handlers and UI updates
*/
/////////////////////////////////////////////////////////////////
#include "UI/ESP32Berry_Display.hpp"
#include "Apps/Settings/ESP32Berry_AppSettings.hpp"
#include <Arduino.h>

extern "C" void wifi_event_cb_thunk(lv_event_t *e);
extern "C" void textarea_event_cb_thunk(lv_event_t *e);
extern "C" void ui_event_callback_thunk(lv_event_t *e);
extern "C" void ui_app_btns_callback_thunk(lv_event_t *e);

void Display::ui_event_callback(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = (lv_obj_t *)lv_event_get_target(e);
    // Any UI event implies user interaction
    register_activity();
    if (target == ui_BasePopupCloseBtn && event_code == LV_EVENT_CLICKED)
    {
        lv_obj_add_flag(ui_BasePopup, LV_OBJ_FLAG_HIDDEN);
    }
}

void Display::ui_app_btns_callback(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {
        set_notification("");

        lv_obj_t *label = lv_obj_get_child(btn, 0);
        // Hidden label inside each app button stores its index as text
        // Forward all indices to the APP handler to allow dynamically added apps.
        lv_scr_load_anim(ui_Sub_Screen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 100, 0, false);
        menu_event_cb(APP, lv_label_get_text(label));
    }
}

void Display::textarea_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);

    if (code == LV_EVENT_FOCUSED)
    {
        ui_Focused_Obj = obj;
    }
    else if (code == LV_EVENT_DEFOCUSED)
    {
        ui_Focused_Obj = NULL;
    }
}

void Display::update_time(void *timeStruct)
{
    lv_port_sem_take();
    char hourMin[10];
    strftime(hourMin, 10, "%H:%M %p", (struct tm *)timeStruct);
    lv_label_set_text(ui_TimeLabel, hourMin);

    char date[12];
    strftime(date, 12, "%a, %b %d", (struct tm *)timeStruct);
    lv_label_set_text(ui_DateLabel, date);
    lv_port_sem_give();
}

void Display::set_notification(const char *msg)
{
    lv_label_set_text(ui_NotiLabel, msg);
}

void Display::update_WiFi_label(void *data1)
{
    lv_port_sem_take();
    if (data1 != NULL)
    {
        std::string &s = *(static_cast<std::string *>(data1));
        s.append(" ");
        s.append(LV_SYMBOL_WIFI);
        lv_label_set_text(ui_WiFiLabel, s.c_str());
    }
    else
    {
        lv_label_set_text(ui_WiFiLabel, LV_SYMBOL_WARNING);
    }
    lv_port_sem_give();
}

void Display::update_battery(void *data1)
{
    int batPercent = *(int *)data1;
    String tempBatString = add_battery_icon(batPercent);
    tempBatString += " ";
    tempBatString += String(batPercent);
    tempBatString += "% ";
    lv_port_sem_take();
    lv_label_set_text(ui_BatteryLabel, tempBatString.c_str());
    lv_port_sem_give();
}

String Display::add_battery_icon(int percentage)
{
    if (percentage >= 90)
    {
        return String(LV_SYMBOL_BATTERY_FULL);
    }
    else if (percentage >= 65 && percentage < 90)
    {
        return String(LV_SYMBOL_BATTERY_3);
    }
    else if (percentage >= 40 && percentage < 65)
    {
        return String(LV_SYMBOL_BATTERY_2);
    }
    else if (percentage >= 15 && percentage < 40)
    {
        return String(LV_SYMBOL_BATTERY_1);
    }
    else
    {
        return String(LV_SYMBOL_BATTERY_EMPTY);
    }
}

lv_obj_t *Display::focused_obj()
{
    return ui_Focused_Obj;
}

void Display::set_focused_obj(lv_obj_t *obj)
{
    ui_Focused_Obj = obj;
}

lv_obj_t *Display::ui_second_screen()
{
    return ui_Sub_Screen;
}

void Display::goback_main_screen()
{
    lv_scr_load_anim(ui_Main_Screen, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 100, 0, false);
}

void Display::lv_port_sem_take(void)
{
    TaskHandle_t task = xTaskGetCurrentTaskHandle();
    if (lvgl_task_handle != task)
    {
        xSemaphoreTake(bin_sem, portMAX_DELAY);
    }
}

void Display::lv_port_sem_give(void)
{
    TaskHandle_t task = xTaskGetCurrentTaskHandle();
    if (lvgl_task_handle != task)
    {
        xSemaphoreGive(bin_sem);
    }
}

int Display::get_display_width()
{
    return tft->width();
}

int Display::get_display_height()
{
    return tft->height();
}
