/////////////////////////////////////////////////////////////////
/*
  New ESP32Berry Project, The base UI & ChatGPT Client
  For More Information: https://youtu.be/5K6rSw9j5iY
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#pragma once
#include "Configurations/ESP32Berry_Config.hpp"
#include "Apps/ESP32Berry_AppBase.hpp"
#include <ArduinoJson.h>
#include <Arduino.h>
#include <TinyGPSPlus.h>

#ifndef SerialGPS
#define SerialGPS Serial1
#endif

#define BOARD_GPS_TX_PIN 43
#define BOARD_GPS_RX_PIN 44
#define BOARD_POWERON 10
static const uint32_t GPSBaud = 9600;

static TinyGPSPlus gps;
class AppGPS : public AppBase
{

private:
  int display_width;

  lv_obj_t *main;
  lv_obj_t *grid_layout = nullptr;
  lv_obj_t *latitude_value;
  lv_obj_t *longitude_value;
  lv_obj_t *satellites_value;
  lv_obj_t *address_value;
  String current_address = "Connecting to GPS...";

public:
  AppGPS(lv_obj_t *screen, const char *title);
  ~AppGPS();
  void draw_ui(); // legacy: kept for compatibility, now calls ensure_ui_created + updates
  void ensure_ui_created();
  void update_ui(double lat, double lon, uint32_t sats, const String &address);
  const String &get_current_address() const { return current_address; }
  void add_msg(bool isMine, String msg);
  void tg_event_handler(lv_event_t *e);
  void close_app();
};