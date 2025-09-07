#include <Arduino.h>
#include "Apps/Settings/ESP32Berry_AppSettings.hpp"
#include "../../UI/ESP32Berry_Display.hpp"
#include "../../Utils/Globals.h"
#include "../../Utils/EventManager/EventManager.h"

static AppSettings *instance = NULL;

extern "C" void ui_settings_event_callback_thunk(lv_event_t *e)
{
  instance->ui_event_callback(e);
}

extern "C" void wifi_event_cb_thunk(lv_event_t *e)
{
  instance->ui_wifi_event_callback(e);
}
extern "C" void textarea_event_cb_thunk(lv_event_t *e);

void AppSettings::ui_wifi_event_callback(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED)
  {
    if (btn == ui_WiFiPanelCloseBtn)
    {
      lv_obj_add_flag(ui_WiFiPanel, LV_OBJ_FLAG_HIDDEN);
    }
    else if (btn == ui_WiFiMBoxConnectBtn)
    {
      lv_obj_add_flag(ui_WiFiMBox, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_WiFiPanel, LV_OBJ_FLAG_HIDDEN);
      char *key = new char[strlen(lv_label_get_text(ui_WiFiMBoxTitle)) + strlen(lv_textarea_get_text(ui_WiFiMBoxPassword)) + 3];
      strcpy(key, lv_label_get_text(ui_WiFiMBoxTitle));
      strcat(key, WIFI_SSID_PW_DELIMITER);
      strcat(key, lv_textarea_get_text(ui_WiFiMBoxPassword));
      // menu_event_cb(WIFI_RADIO_ON, key);
      GlobalEventBus.emit(Events::SET_WIFI, Network_Event_t::NETWORK_CONNECTING, key);
      delete[] key;
      this->show_loading_popup(true);
      lv_textarea_set_text(ui_WiFiMBoxPassword, "");
    }
    else if (btn == ui_WiFiMBoxCloseBtn)
    {
      lv_obj_move_background(ui_WiFiMBox);
      lv_obj_add_flag(ui_WiFiMBox, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
      int length = strlen(lv_list_get_btn_text(ui_WiFiList, btn));
      char ssidName[length];
      strncpy(ssidName, lv_list_get_btn_text(ui_WiFiList, btn), length);
      ssidName[length] = '\0';
      lv_label_set_text(ui_WiFiMBoxTitle, ssidName);
      lv_obj_move_foreground(ui_WiFiMBox);
      lv_obj_clear_flag(ui_WiFiMBox, LV_OBJ_FLAG_HIDDEN);
    }
  }
}

extern "C" void settings_async_wifi(void *data)
{
  if (!instance)
    return;
  instance->ui_WiFi_page();
  instance->show_loading_popup(false);
}

void AppSettings::update_ui_network(void *data1, void *data2)
{

  GlobalEventBus.emit(Events::PORT_SEM_TAKE, "");
  if (!lv_obj_has_flag(ui_WiFiMBox, LV_OBJ_FLAG_HIDDEN))
  {
    GlobalEventBus.emit(Events::PORT_SEM_GIVE, "");
    return;
  }

  lv_obj_clear_flag(ui_WiFiList, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(ui_WiFiList, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_clean(ui_WiFiList);

  int *arraySize = static_cast<int *>(data2);

  std::string *strPtr = static_cast<std::string *>(data1);
  std::vector<std::string> newWifiList(strPtr, strPtr + *arraySize);

  lv_list_add_text(ui_WiFiList, newWifiList.size() > 1 ? "WiFi: Found Networks" : "WiFi: Not Found!");
  for (std::vector<std::string>::iterator item = newWifiList.begin(); item != newWifiList.end(); ++item)
  {

    lv_obj_t *btn = lv_list_add_btn(ui_WiFiList, LV_SYMBOL_WIFI, (*item).c_str());
    lv_obj_add_event_cb(btn, wifi_event_cb_thunk, LV_EVENT_CLICKED, NULL);
  }
  GlobalEventBus.emit(Events::PORT_SEM_GIVE, "");

  lv_obj_add_flag(ui_WiFiList, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_flag(ui_WiFiList, LV_OBJ_FLAG_SCROLLABLE);
}

AppSettings::AppSettings(Display *display, System *system, Network *network, const char *title)
    : AppBase(display, system, network, title)
{
  instance = this;

  Globals::get().preferences.begin("settings", true);
  pendingVolume = Globals::get().preferences.getInt("volume", 21);
  wiFiStateOn = Globals::get().preferences.getBool("wifi_state", true);
  screen_brightness = Globals::get().preferences.getInt("brightness", 200);
  Globals::get().preferences.end();

  this->draw_ui();
}

AppSettings::~AppSettings()
{
}

void AppSettings::draw_ui()
{
  LV_IMG_DECLARE(icon_brightness);
  LV_IMG_DECLARE(icon_speaker);
  LV_IMG_DECLARE(icon_wifi);
  LV_IMG_DECLARE(icon_chatgpt);

  lv_obj_t *ui_LabelScreenTimeout = lv_label_create(ui_AppPanel);
  lv_obj_set_width(ui_LabelScreenTimeout, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_LabelScreenTimeout, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_LabelScreenTimeout, 0);
  lv_obj_set_y(ui_LabelScreenTimeout, -4);
  lv_label_set_text(ui_LabelScreenTimeout, "Screen Timeout (min):");
  lv_obj_set_style_text_color(ui_LabelScreenTimeout, lv_color_hex(0x3D3D3D), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_LabelScreenTimeout, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_LabelScreenTimeout, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_LabelScreenTimeoutValue = lv_label_create(ui_AppPanel);
  lv_obj_set_width(ui_LabelScreenTimeoutValue, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_LabelScreenTimeoutValue, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_LabelScreenTimeoutValue, 140);
  lv_obj_set_y(ui_LabelScreenTimeoutValue, -4);
  lv_label_set_text(ui_LabelScreenTimeoutValue, "5");
  lv_obj_set_style_text_color(ui_LabelScreenTimeoutValue, lv_color_hex(0x3D3D3D), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_LabelScreenTimeoutValue, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ScreenTimeout = lv_slider_create(ui_AppPanel);
  lv_slider_set_range(ui_ScreenTimeout, 1, 60);
  lv_obj_set_width(ui_ScreenTimeout, 280);
  lv_obj_set_height(ui_ScreenTimeout, 8);
  lv_obj_set_x(ui_ScreenTimeout, 10);
  lv_obj_set_y(ui_ScreenTimeout, 22);
  lv_obj_set_style_bg_color(ui_ScreenTimeout, lv_color_hex(0x989898), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ScreenTimeout, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ScreenTimeout, lv_color_hex(0xE95622), LV_PART_INDICATOR | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ScreenTimeout, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ScreenTimeout, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ScreenTimeout, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(ui_ScreenTimeout, lv_color_hex(0x000000), LV_PART_KNOB | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(ui_ScreenTimeout, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ScreenTimeout, 1, LV_PART_KNOB | LV_STATE_DEFAULT);
  lv_slider_set_value(ui_ScreenTimeout, 1, LV_ANIM_OFF);
  lv_obj_add_event_cb(ui_ScreenTimeout, ui_settings_event_callback_thunk, LV_EVENT_VALUE_CHANGED, NULL);

  lv_obj_t *ui_IconBrightness = lv_img_create(ui_AppPanel);
  lv_img_set_src(ui_IconBrightness, &icon_brightness);
  lv_obj_set_width(ui_IconBrightness, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_IconBrightness, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_IconBrightness, 0);
  lv_obj_set_y(ui_IconBrightness, 40);
  lv_obj_add_flag(ui_IconBrightness, LV_OBJ_FLAG_ADV_HITTEST);
  lv_obj_clear_flag(ui_IconBrightness, LV_OBJ_FLAG_SCROLLABLE);

  ui_SliderBrightness = lv_slider_create(ui_AppPanel);
  lv_slider_set_range(ui_SliderBrightness, 10, 255);
  lv_obj_set_width(ui_SliderBrightness, 250);
  lv_obj_set_height(ui_SliderBrightness, 8);
  lv_obj_set_x(ui_SliderBrightness, 32);
  lv_obj_set_y(ui_SliderBrightness, 44);
  lv_obj_set_style_bg_color(ui_SliderBrightness, lv_color_hex(0x989898), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_SliderBrightness, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_SliderBrightness, lv_color_hex(0xE95622), LV_PART_INDICATOR | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_SliderBrightness, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_SliderBrightness, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_SliderBrightness, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(ui_SliderBrightness, lv_color_hex(0x000000), LV_PART_KNOB | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(ui_SliderBrightness, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_SliderBrightness, 1, LV_PART_KNOB | LV_STATE_DEFAULT);
  lv_slider_set_value(ui_SliderBrightness, screen_brightness, LV_ANIM_OFF);
  lv_obj_add_event_cb(ui_SliderBrightness, ui_settings_event_callback_thunk, LV_EVENT_VALUE_CHANGED, NULL);

  lv_obj_t *ui_IconSpeaker = lv_img_create(ui_AppPanel);
  lv_img_set_src(ui_IconSpeaker, &icon_speaker);
  lv_obj_set_width(ui_IconSpeaker, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_IconSpeaker, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_IconSpeaker, 0);
  lv_obj_set_y(ui_IconSpeaker, 60);
  lv_obj_add_flag(ui_IconSpeaker, LV_OBJ_FLAG_ADV_HITTEST);
  lv_obj_clear_flag(ui_IconSpeaker, LV_OBJ_FLAG_SCROLLABLE);

  ui_SliderSpeaker = lv_slider_create(ui_AppPanel);
  lv_slider_set_range(ui_SliderSpeaker, 0, 21);
  lv_obj_set_width(ui_SliderSpeaker, 250);
  lv_obj_set_height(ui_SliderSpeaker, 8);
  lv_obj_set_x(ui_SliderSpeaker, 32);
  lv_obj_set_y(ui_SliderSpeaker, 64);
  lv_obj_set_style_bg_color(ui_SliderSpeaker, lv_color_hex(0x989898), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_SliderSpeaker, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_set_style_bg_color(ui_SliderSpeaker, lv_color_hex(0xE95622), LV_PART_INDICATOR | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_SliderSpeaker, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);

  lv_obj_set_style_bg_color(ui_SliderSpeaker, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_SliderSpeaker, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(ui_SliderSpeaker, lv_color_hex(0x000000), LV_PART_KNOB | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(ui_SliderSpeaker, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_SliderSpeaker, 1, LV_PART_KNOB | LV_STATE_DEFAULT);
  lv_slider_set_value(ui_SliderSpeaker, pendingVolume, LV_ANIM_OFF);
  lv_obj_add_event_cb(ui_SliderSpeaker, ui_settings_event_callback_thunk, LV_EVENT_VALUE_CHANGED, NULL);

  lv_obj_t *ui_PanelWifi = lv_obj_create(ui_AppPanel);
  lv_obj_set_width(ui_PanelWifi, 100);
  lv_obj_set_height(ui_PanelWifi, 30);
  lv_obj_set_align(ui_PanelWifi, LV_ALIGN_BOTTOM_LEFT);
  lv_obj_clear_flag(ui_PanelWifi, LV_OBJ_FLAG_SCROLLABLE);

  ui_BtnWiFi = lv_btn_create(ui_PanelWifi);
  lv_obj_set_width(ui_BtnWiFi, 60);
  lv_obj_set_height(ui_BtnWiFi, 50);
  lv_obj_set_x(ui_BtnWiFi, 16);
  lv_obj_set_y(ui_BtnWiFi, 0);
  lv_obj_set_align(ui_BtnWiFi, LV_ALIGN_LEFT_MID);
  lv_obj_add_flag(ui_BtnWiFi, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
  lv_obj_clear_flag(ui_BtnWiFi, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(ui_BtnWiFi, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_BtnWiFi, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_event_cb(ui_BtnWiFi, ui_settings_event_callback_thunk, LV_EVENT_CLICKED, NULL);

  lv_obj_t *ui_Label2 = lv_label_create(ui_BtnWiFi);
  lv_obj_set_width(ui_Label2, 60);
  lv_obj_set_height(ui_Label2, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_Label2, -10);
  lv_obj_set_y(ui_Label2, 0);
  lv_obj_set_align(ui_Label2, LV_ALIGN_LEFT_MID);
  lv_label_set_text(ui_Label2, "WiFi");
  lv_obj_set_style_text_color(ui_Label2, lv_color_hex(0x3D3D3D), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_Label2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_Label2, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ImgBtnWiFi = lv_imgbtn_create(ui_PanelWifi);
  lv_imgbtn_set_src(ui_ImgBtnWiFi, LV_IMGBTN_STATE_RELEASED, NULL, &icon_wifi, NULL);
  lv_imgbtn_set_src(ui_ImgBtnWiFi, LV_IMGBTN_STATE_PRESSED, NULL, &icon_wifi, NULL);
  lv_imgbtn_set_src(ui_ImgBtnWiFi, LV_IMGBTN_STATE_DISABLED, NULL, &icon_wifi, NULL);
  lv_imgbtn_set_src(ui_ImgBtnWiFi, LV_IMGBTN_STATE_CHECKED_PRESSED, NULL, &icon_wifi, NULL);
  lv_imgbtn_set_src(ui_ImgBtnWiFi, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &icon_wifi, NULL);
  lv_imgbtn_set_src(ui_ImgBtnWiFi, LV_IMGBTN_STATE_CHECKED_DISABLED, NULL, &icon_wifi, NULL);
  lv_obj_set_width(ui_ImgBtnWiFi, 20);
  lv_obj_set_height(ui_ImgBtnWiFi, 20);
  lv_obj_set_x(ui_ImgBtnWiFi, -10);
  lv_obj_set_y(ui_ImgBtnWiFi, 0);
  lv_obj_set_align(ui_ImgBtnWiFi, LV_ALIGN_LEFT_MID);
  lv_obj_add_flag(ui_ImgBtnWiFi, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_set_style_radius(ui_ImgBtnWiFi, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ImgBtnWiFi, lv_color_hex(0xE95622), LV_PART_MAIN | LV_STATE_CHECKED);
  lv_obj_set_style_bg_opa(ui_ImgBtnWiFi, 255, LV_PART_MAIN | LV_STATE_CHECKED);
  lv_obj_add_event_cb(ui_ImgBtnWiFi, ui_settings_event_callback_thunk, LV_EVENT_CLICKED, NULL);
  if (wiFiStateOn)
    lv_obj_clear_state(ui_ImgBtnWiFi, LV_STATE_CHECKED);
  else
    lv_obj_add_state(ui_ImgBtnWiFi, LV_STATE_CHECKED);
  // Removed: ui_TopPanel is not in scope here

  ui_PanelCursor = lv_obj_create(ui_AppPanel);
  lv_obj_set_width(ui_PanelCursor, 30);
  lv_obj_set_height(ui_PanelCursor, 30);
  lv_obj_set_align(ui_PanelCursor, LV_ALIGN_BOTTOM_RIGHT);
  lv_obj_clear_flag(ui_PanelCursor, LV_OBJ_FLAG_SCROLLABLE);

  ui_ImgBtnCursor = lv_imgbtn_create(ui_PanelCursor);
  lv_imgbtn_set_src(ui_ImgBtnCursor, LV_IMGBTN_STATE_RELEASED, NULL, &mouse_cursor_icon, NULL);
  lv_imgbtn_set_src(ui_ImgBtnCursor, LV_IMGBTN_STATE_PRESSED, NULL, &mouse_cursor_icon, NULL);
  lv_imgbtn_set_src(ui_ImgBtnCursor, LV_IMGBTN_STATE_DISABLED, NULL, &mouse_cursor_icon, NULL);
  lv_imgbtn_set_src(ui_ImgBtnCursor, LV_IMGBTN_STATE_CHECKED_PRESSED, NULL, &mouse_cursor_icon, NULL);
  lv_imgbtn_set_src(ui_ImgBtnCursor, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &mouse_cursor_icon, NULL);
  lv_imgbtn_set_src(ui_ImgBtnCursor, LV_IMGBTN_STATE_CHECKED_DISABLED, NULL, &mouse_cursor_icon, NULL);
  lv_obj_set_width(ui_ImgBtnCursor, mouse_cursor_icon.header.w);
  lv_obj_set_height(ui_ImgBtnCursor, mouse_cursor_icon.header.h);
  lv_obj_clear_flag(ui_ImgBtnCursor, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_align(ui_ImgBtnCursor, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_ImgBtnCursor, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_set_style_radius(ui_ImgBtnCursor, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ImgBtnCursor, 255, LV_PART_MAIN | LV_STATE_CHECKED);
  lv_obj_add_event_cb(ui_ImgBtnCursor, ui_settings_event_callback_thunk, LV_EVENT_CLICKED, NULL);

  // ui_WiFi_page();
  // show_loading_popup(true);
  // lv_async_call(settings_async_wifi, NULL);
}

void AppSettings::ui_WiFi_page()
{
  ui_WiFiPanel = lv_obj_create(ui_AppPanel);
  lv_obj_set_size(ui_WiFiPanel, Globals::get().screen_width - 40, Globals::get().screen_height - 60);
  lv_obj_align(ui_WiFiPanel, LV_ALIGN_TOP_MID, 0, 0);

  lv_obj_t *ui_WiFiPanelLabel = lv_label_create(ui_WiFiPanel);
  lv_label_set_text(ui_WiFiPanelLabel, "Wi-Fi " LV_SYMBOL_SETTINGS);
  lv_obj_align(ui_WiFiPanelLabel, LV_ALIGN_TOP_LEFT, 0, 0);

  ui_WiFiPanelCloseBtn = lv_btn_create(ui_WiFiPanel);
  lv_obj_set_size(ui_WiFiPanelCloseBtn, 30, 30);
  lv_obj_align(ui_WiFiPanelCloseBtn, LV_ALIGN_TOP_RIGHT, 0, -10);
  lv_obj_set_style_bg_color(ui_WiFiPanelCloseBtn, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_WiFiPanelCloseBtn, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(ui_WiFiPanelCloseBtn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_WiFiPanelCloseBtn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_width(ui_WiFiPanelCloseBtn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_spread(ui_WiFiPanelCloseBtn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_event_cb(ui_WiFiPanelCloseBtn, wifi_event_cb_thunk, LV_EVENT_CLICKED, NULL);

  lv_obj_t *ui_CloseBtnSymbol = lv_label_create(ui_WiFiPanelCloseBtn);
  lv_label_set_text(ui_CloseBtnSymbol, LV_SYMBOL_CLOSE);
  lv_obj_center(ui_CloseBtnSymbol);
  lv_obj_set_style_text_color(ui_CloseBtnSymbol, lv_color_hex(0x3D3D3D), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_CloseBtnSymbol, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_WiFiList = lv_list_create(ui_WiFiPanel);
  lv_obj_set_size(ui_WiFiList, Globals::get().screen_width - 70, Globals::get().screen_height - 105);
  lv_obj_align_to(ui_WiFiList, ui_WiFiPanelLabel, LV_ALIGN_TOP_LEFT, 0, 20);

  // Populate the list (scan and add items)
  lv_obj_clean(ui_WiFiList);
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i)
  {
    String ssid = WiFi.SSID(i);
    int32_t rssi = WiFi.RSSI(i);
    bool locked = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;

    char label[96];
    // Use a simple ASCII marker for secured networks to avoid relying on an undefined LVGL symbol
    snprintf(label, sizeof(label), "%s", ssid.c_str());

    lv_obj_t *item = lv_list_add_btn(ui_WiFiList, LV_SYMBOL_WIFI, label);
    lv_obj_add_event_cb(item, wifi_event_cb_thunk, LV_EVENT_CLICKED, NULL);
  }

  // lv_obj_add_flag(ui_WiFiPanel, LV_OBJ_FLAG_HIDDEN);

  ui_WiFiMBox = lv_obj_create(ui_AppPanel);
  lv_obj_set_size(ui_WiFiMBox, Globals::get().screen_width - 40, Globals::get().screen_height - 80);
  lv_obj_center(ui_WiFiMBox);

  lv_obj_t *mboxLabel = lv_label_create(ui_WiFiMBox);
  lv_label_set_text(mboxLabel, "Selected WiFi SSID");
  lv_obj_set_size(mboxLabel, Globals::get().screen_width - 70, 40);
  lv_obj_align(mboxLabel, LV_ALIGN_TOP_LEFT, 0, 0);

  ui_WiFiMBoxTitle = lv_label_create(ui_WiFiMBox);
  lv_label_set_text(ui_WiFiMBoxTitle, "ThatProject");
  lv_obj_set_size(ui_WiFiMBoxTitle, Globals::get().screen_width - 70, 40);
  lv_obj_align(ui_WiFiMBoxTitle, LV_ALIGN_TOP_LEFT, 0, 30);

  ui_WiFiMBoxPassword = lv_textarea_create(ui_WiFiMBox);
  lv_textarea_set_cursor_click_pos(ui_WiFiMBoxPassword, false);
  lv_textarea_set_cursor_pos(ui_WiFiMBoxPassword, 0);
  lv_textarea_set_text_selection(ui_WiFiMBoxPassword, false);
  lv_obj_set_size(ui_WiFiMBoxPassword, Globals::get().screen_width - 70, 40);
  lv_obj_align_to(ui_WiFiMBoxPassword, ui_WiFiMBoxTitle, LV_ALIGN_TOP_LEFT, 0, 20);
  lv_textarea_set_placeholder_text(ui_WiFiMBoxPassword, "Password?");
  lv_textarea_set_max_length(ui_WiFiMBoxPassword, 64);
  lv_obj_add_event_cb(ui_WiFiMBoxPassword, textarea_event_cb_thunk, LV_EVENT_FOCUSED, NULL);
  lv_obj_add_event_cb(ui_WiFiMBoxPassword, textarea_event_cb_thunk, LV_EVENT_DEFOCUSED, NULL);

  ui_WiFiMBoxConnectBtn = lv_btn_create(ui_WiFiMBox);
  lv_obj_add_event_cb(ui_WiFiMBoxConnectBtn, wifi_event_cb_thunk, LV_EVENT_ALL, NULL);
  lv_obj_align(ui_WiFiMBoxConnectBtn, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
  lv_obj_t *btnLabel = lv_label_create(ui_WiFiMBoxConnectBtn);
  lv_label_set_text(btnLabel, "Connect");
  lv_obj_center(btnLabel);

  ui_WiFiMBoxCloseBtn = lv_btn_create(ui_WiFiMBox);
  lv_obj_add_event_cb(ui_WiFiMBoxCloseBtn, wifi_event_cb_thunk, LV_EVENT_ALL, NULL);
  lv_obj_align(ui_WiFiMBoxCloseBtn, LV_ALIGN_BOTTOM_LEFT, 0, 0);
  lv_obj_t *btnLabel2 = lv_label_create(ui_WiFiMBoxCloseBtn);
  lv_label_set_text(btnLabel2, "Cancel");
  lv_obj_center(btnLabel2);

  lv_obj_add_flag(ui_WiFiMBox, LV_OBJ_FLAG_HIDDEN);
}

void AppSettings::ui_event_callback(lv_event_t *e)
{
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *target = (lv_obj_t *)lv_event_get_target(e);
  instance->ui_interactive();
  if (target == ui_SliderBrightness && event_code == LV_EVENT_VALUE_CHANGED)
  {
    int sliderValue = lv_slider_get_value(ui_SliderBrightness);

    GlobalEventBus.emit(Events::BRIGHTNESS_CHANGED, String(sliderValue));
    // Remember user's chosen brightness so inactivity wake restores this
    previous_brightness = sliderValue;
    if (screen_dimmed && sliderValue > 0)
    {
      // If user moves slider while dimmed, treat as activity and undim immediately
      screen_dimmed = false;
    }
  }
  else if (target == ui_SliderSpeaker && event_code == LV_EVENT_VALUE_CHANGED)
  {
    int sliderValue = lv_slider_get_value(ui_SliderSpeaker);
    GlobalEventBus.emit(Events::VOLUME_CHANGED, String(sliderValue));
    pendingVolume = sliderValue;
  }
  else if (target == ui_ImgBtnWiFi && event_code == LV_EVENT_CLICKED)
  {
    if (wiFiStateOn)
    {
      GlobalEventBus.emit(Events::SET_WIFI, Network_Event_t::NETWORK_DISCONNECTED, NULL);
      lv_obj_set_state(ui_ImgBtnWiFi, LV_STATE_CHECKED, true);
      wiFiStateOn = false;
    }
    else
    {
      lv_obj_clear_state(ui_ImgBtnWiFi, LV_STATE_CHECKED);
      GlobalEventBus.emit(Events::SET_WIFI, Network_Event_t::NETWORK_CONNECTED, NULL);
      wiFiStateOn = true;
    }
  }
  else if (target == ui_BtnWiFi && event_code == LV_EVENT_CLICKED)
  {
    ui_WiFi_page();
  }
  else if ((target == ui_ImgBtnCursor && event_code == LV_EVENT_CLICKED) || (target == ui_PanelCursor && event_code == LV_EVENT_CLICKED))
  {
    // Toggles each time
    Globals::get().cursor_panel_active = !Globals::get().cursor_panel_active;
    if (Globals::get().cursor_panel_active)
    {
      lv_obj_set_style_bg_color(ui_PanelCursor, lv_color_hex(0x33bd33), LV_PART_MAIN | LV_STATE_DEFAULT); // #33bd33ff
      lv_obj_set_style_bg_color(ui_ImgBtnCursor, lv_color_hex(0x33bd33), LV_PART_MAIN | LV_STATE_DEFAULT);
      GlobalEventBus.emit(Events::TOGGLE_CURSOR, "on");
    }
    else
    {
      lv_obj_set_style_bg_color(ui_PanelCursor, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_bg_color(ui_ImgBtnCursor, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
      GlobalEventBus.emit(Events::TOGGLE_CURSOR, "off");
    }
  }
  else if (target == ui_ScreenTimeout && event_code == LV_EVENT_VALUE_CHANGED)
  {
    int sliderValue = lv_slider_get_value(ui_ScreenTimeout);
    String timeoutText = String(sliderValue) + " min";
    lv_label_set_text(ui_LabelScreenTimeoutValue, timeoutText.c_str());
    GlobalEventBus.emit(Events::SCREEN_TIMEOUT_CHANGED, String(sliderValue));
  }
}

void AppSettings::close_app()
{
  _display->goback_main_screen();
  lv_obj_del(_bodyScreen);
  delete this;
}
