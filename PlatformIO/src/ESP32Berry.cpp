#include "ESP32Berry.hpp"
#include "Utils/Globals.h"
#include "Utils/EventManager/EventManager.h"

static ESP32Berry *instance = NULL;

ESP32Berry::ESP32Berry()
{
  instance = this;
}

ESP32Berry::~ESP32Berry() {}

void displayEventHandler(Menu_Event_t event, void *param)
{
  switch (event)
  {
  case APP:
  {
    int menuNum = std::stoi((char *)param);
    switch (menuNum)
    {
    case 0:
      instance->appChatGPT = new AppChatGPT(instance->display, instance->system, instance->network, "ChatGPT Client");
      break;
    case 2:
      instance->appFileBrowser = new AppFileBrowser(instance->display, instance->system, instance->network, "File Browser");
      break;
    case 1:
      instance->appGPS = new AppGPS(instance->display, instance->system, instance->network, "GPS Client");
      break;
    case 4:
      instance->appSettings = new AppSettings(instance->display, instance->system, instance->network, "Settings");
      break;
    }

    break;
  }
  }
}

void networkResponse(Network_Event_t event, void *data1, void *data2)
{
  switch (event)
  {
  case NETWORK_SCANNING_ON:
    // TODO: switch to event based async
    instance->appSettings->update_ui_network(data1, data2);
    break;

  case NETWORK_CONNECTED:
    instance->system->setConfigTzTime();
    instance->display->show_loading_popup(false);
    instance->display->set_notification("[WiFi] Connected!");
    instance->display->update_WiFi_label(data1);
    break;

  case NETWORK_CONNECT_FAILURE:
  case NETWORK_DISCONNECTED:
    instance->display->show_loading_popup(false);
    instance->display->set_notification("[WIFI] Unable to connect to selected WiFi.");
    instance->display->update_WiFi_label(data1);
    break;
  }
}

void systemInfo(System_Event_t event, void *param)
{
  switch (event)
  {
  case SYS_TIME:
    instance->display->update_time(param);
    break;

  case SYS_BATTERY:
    instance->display->update_battery(param);
    break;
  }
}

void ESP32Berry::begin()
{
  void (*iptr)(Menu_Event_t, void *) = &displayEventHandler;
  display = new Display(displayEventHandler);

  void (*vptr)(Network_Event_t, void *, void *) = &networkResponse;
  network = new Network(networkResponse);

  void (*sptr)(System_Event_t, void *) = &systemInfo;
  system = new System(systemInfo);

  Globals::get().preferences.begin("settings", true);
  int volume = Globals::get().preferences.getInt("volume", 21);
  int screenTimeout = Globals::get().preferences.getInt("screen_timeout", 1);
  Globals::get().preferences.end();
  instance->system->play_audio(AUDIO_BOOT);
  GlobalEventBus.emit(Events::VOLUME_CHANGED, String(volume));
  instance->display->set_screen_timeout(screenTimeout);
}