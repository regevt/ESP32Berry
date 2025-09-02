/////////////////////////////////////////////////////////////////
/*
  WiFi panel and message box UI
*/
/////////////////////////////////////////////////////////////////
#include "UI/ESP32Berry_Display.hpp"
#include <WiFi.h>  // add this

extern "C" void wifi_event_cb_thunk(lv_event_t *e);
extern "C" void textarea_event_cb_thunk(lv_event_t *e);

void Display::ui_wifi_event_callback(lv_event_t *e)
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
      menu_event_cb(WIFI_ON, key);
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

void Display::ui_WiFi_page()
{
  ui_WiFiPanel = lv_obj_create(ui_Main_Screen);
  lv_obj_set_size(ui_WiFiPanel, tft->width() - 40, tft->height() - 40);
  lv_obj_align(ui_WiFiPanel, LV_ALIGN_TOP_MID, 0, 20);

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
  lv_obj_set_size(ui_WiFiList, tft->width() - 70, tft->height() - 90);
  lv_obj_align_to(ui_WiFiList, ui_WiFiPanelLabel, LV_ALIGN_TOP_LEFT, 0, 20);

  // Populate the list (scan and add items)
  lv_obj_clean(ui_WiFiList);
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i) {
    String ssid = WiFi.SSID(i);
    int32_t rssi = WiFi.RSSI(i);
    bool locked = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;

  char label[96];
  // Use a simple ASCII marker for secured networks to avoid relying on an undefined LVGL symbol
  snprintf(label, sizeof(label), "%s", ssid.c_str());

    lv_obj_t *item = lv_list_add_btn(ui_WiFiList, LV_SYMBOL_WIFI, label);
    lv_obj_add_event_cb(item, wifi_event_cb_thunk, LV_EVENT_CLICKED, NULL);
  }

  // If you intend to show the panel immediately, don't hide it here.
  // If you want it hidden by default, make sure you clear this flag when opening the Wi-Fi page.
  lv_obj_add_flag(ui_WiFiPanel, LV_OBJ_FLAG_HIDDEN);

  ui_WiFiMBox = lv_obj_create(ui_Main_Screen);
  lv_obj_set_size(ui_WiFiMBox, tft->width() - 40, tft->height() - 80);
  lv_obj_center(ui_WiFiMBox);

  lv_obj_t *mboxLabel = lv_label_create(ui_WiFiMBox);
  lv_label_set_text(mboxLabel, "Selected WiFi SSID");
  lv_obj_set_size(mboxLabel, tft->width() - 70, 40);
  lv_obj_align(mboxLabel, LV_ALIGN_TOP_LEFT, 0, 0);

  ui_WiFiMBoxTitle = lv_label_create(ui_WiFiMBox);
  lv_label_set_text(ui_WiFiMBoxTitle, "ThatProject");
  lv_obj_set_size(ui_WiFiMBoxTitle, tft->width() - 70, 40);
  lv_obj_align(ui_WiFiMBoxTitle, LV_ALIGN_TOP_LEFT, 0, 30);

  ui_WiFiMBoxPassword = lv_textarea_create(ui_WiFiMBox);
  lv_textarea_set_cursor_click_pos(ui_WiFiMBoxPassword, false);
  lv_textarea_set_cursor_pos(ui_WiFiMBoxPassword, 0);
  lv_textarea_set_text_selection(ui_WiFiMBoxPassword, false);
  lv_obj_set_size(ui_WiFiMBoxPassword, tft->width() - 70, 40);
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
