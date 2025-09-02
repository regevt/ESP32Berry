#pragma once
#include "Configurations/ESP32Berry_Config.hpp"

#include <vector>
#include "UI/ESP32Berry_Display.hpp"
#include "ESP32Berry_Network.hpp"
#include "ESP32Berry_System.hpp"
#include "Apps/ChatGPT/ESP32Berry_AppChatGPT.hpp"
#include "Apps/FileBrowser/ESP32Berry_AppFileBrowser.hpp"

class ESP32Berry
{
private:
public:
  Display *display;
  Network *network;
  System *system;
  AppChatGPT *appChatGPT;
  AppFileBrowser *appFileBrowser;

  ESP32Berry();
  ~ESP32Berry();
  void begin();
};