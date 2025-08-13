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
#include <ChatGPT.hpp>
#include <Arduino.h>
#include <TinyGPSPlus.h>

static TinyGPSPlus gps;
class AppGPS : public AppBase
{

private:
  int display_width;

public:
  AppGPS(Display *display, System *system, Network *network, const char *title);
  ~AppGPS();
  void draw_ui();
  void add_msg(bool isMine, String msg);
  void tg_event_handler(lv_event_t *e);
  void close_app();
};