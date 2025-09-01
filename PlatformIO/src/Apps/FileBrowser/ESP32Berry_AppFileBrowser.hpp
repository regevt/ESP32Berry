#pragma once
#include "Configurations/ESP32Berry_Config.hpp"
#include "Apps/ESP32Berry_AppBase.hpp"
#include <ArduinoJson.h>
#include <Arduino.h>
#include <vector>

class AppFileBrowser : public AppBase
{
private:
  int display_width;
  WiFiClientSecure client;

  void initFileBrowser();

public:
  AppFileBrowser(Display *display, System *system, Network *network, const char *title);
  ~AppFileBrowser();

  std::vector<string> history;
  string currentDir = "/";
  void printFiles(string rootDir);
  void clean_input_field();
  void close_app();
};