#pragma once
#include "Configurations/ESP32Berry_Config.hpp"
#include "Apps/ESP32Berry_AppBase.hpp"
#include <Arduino.h>

class AppSettings : public AppBase
{
private:
  int pendingVolume = -1;
  bool wiFiStateOn = false;
  int32_t screen_brightness;
  lv_obj_t *ui_SliderBrightness;
  lv_obj_t *ui_ImgBtnWiFi;
  lv_obj_t *ui_BtnWiFi;
  lv_obj_t *ui_ImgBtnCursor;
  lv_obj_t *ui_PanelCursor;
  FuncPtrInt menu_event_cb;

  lv_obj_t *ui_WiFiMBoxTitle;
  lv_obj_t *ui_WiFiMBoxPassword;
  lv_obj_t *ui_WiFiMBoxConnectBtn;
  lv_obj_t *ui_WiFiMBoxCloseBtn;
  lv_obj_t *ui_WiFiPanelCloseBtn;
  lv_obj_t *ui_WiFiPanel;

  int screen_timeout_minutes = 5;    // Configured timeout (minutes); 0 means disabled
  bool screen_dimmed = false;        // Whether we've turned the backlight off due to inactivity
  uint8_t previous_brightness = 240; // Brightness to restore after wake
  void toggleCursor(lv_event_t *e);
  void textarea_event_cb(lv_event_t *e);

public:
  AppSettings(Display *display, System *system, Network *network, const char *title);
  ~AppSettings();
  void draw_ui();
  lv_obj_t *ui_SliderSpeaker;
  lv_obj_t *ui_LabelScreenTimeoutValue;
  lv_obj_t *ui_ScreenTimeout;
  lv_obj_t *ui_TopPanel = nullptr;
  lv_obj_t *ui_WiFiMBox;
  lv_obj_t *ui_WiFiList;
  void ui_wifi_event_callback(lv_event_t *e);
  void ui_event_callback(lv_event_t *e);
  void update_ui_network(void *data1, void *data2);
  void ui_WiFi_page();
  void close_app();
};
