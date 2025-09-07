#include "ESP32Berry_Network.hpp"
#include "Utils/Globals.h"
#include "Utils/EventManager/EventManager.h"

static Network *instance = NULL;

extern "C" void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info)
{
  instance->WiFiEvent(event, info);
}

extern "C" void WiFiEventOn()
{
  WiFi.onEvent(WiFiEvent);
}

void ntBeginTask(void *pvParameters)
{
  WiFi.mode(WIFI_STA);
  vTaskDelay(100);
  WiFi.begin(instance->_ssid.c_str(), instance->_pwd.c_str());
  vTaskDelete(NULL);
}

Network::Network(FuncPtrVector callback)
{
  instance = this;
  ntScanTaskHandler = NULL;
  ntConnectTaskHandler = NULL;
  network_result_cb = callback;
  _networkEvent = NETWORK_DISCONNECTED;
  loadWiFiDetails(_ssid, _pwd);
  if (wifiSate && _ssid.length() > 0 && _pwd.length() > 0)
  {
    xTaskCreate(ntBeginTask, "ntBeginTask", 4096, NULL, 1, &ntConnectTaskHandler);
  }

  WiFiEventOn();

  GlobalEventBus.on(Events::SET_WIFI, [](int evt, void *param)
                    { instance->WiFiCommend(static_cast<Network_Event_t>(evt), param); });
}

Network::~Network()
{
}

void Network::WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info)
{
  std::string temp;

  switch (event)
  {
  case SYSTEM_EVENT_STA_DISCONNECTED:
    this->network_result_cb(NETWORK_DISCONNECTED, NULL, NULL);
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    temp = WiFi.localIP().toString().c_str();
    this->network_result_cb(NETWORK_CONNECTED, static_cast<void *>(&(temp)), NULL);
    break;
  default:
    break;
  }
}

void ntScanTask(void *pvParam)
{
  int taskCount = 0;
  std::vector<std::string> foundWifiList;
  while (1)
  {
    ++taskCount;
    foundWifiList.clear();
    int n = WiFi.scanNetworks();
    vTaskDelay(10);
    for (int i = 0; i < n; ++i)
    {
      String item = WiFi.SSID(i) + " (" + WiFi.RSSI(i) + ") " + ((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
      foundWifiList.push_back(item.c_str());
      vTaskDelay(10);
    }

    instance->network_result_cb(NETWORK_SCANNING_ON, static_cast<void *>(foundWifiList.data()), &n);
    vTaskDelay(5000);

    if (taskCount >= WIFI_SCAN_ITER)
    {
      instance->ntScanTaskHandler = NULL;
      vTaskDelete(NULL);
    }
  }
}

void Network::WiFiCommend(Network_Event_t networkEvent, void *param)
{
  switch (networkEvent)
  {
  case NETWORK_SCANNING_OFF:
    Serial.println("Stopping WiFi Scan...");
    this->WiFiScanner(false);
    break;
  case NETWORK_SCANNING_ON:
    Serial.println("Starting WiFi Scan...");
    this->WiFiScanner(true);
    break;
  case NETWORK_CONNECTING:
    this->WiFiConnector(param);
    break;
  case NETWORK_DISCONNECTED:
    WiFi.disconnect();
    Serial.println("WiFi Disconnected");
    Globals::get().preferences.begin("settings", false);
    Globals::get().preferences.putBool("wifi_state", false);
    Globals::get().preferences.end();
    break;
  case NETWORK_CONNECTED:
    WiFi.reconnect();
    Serial.println("WiFi Reconnecting...");
    Globals::get().preferences.begin("settings", false);
    Globals::get().preferences.putBool("wifi_state", true);
    Globals::get().preferences.end();
    break;
  }
}

void Network::WiFiScanner(bool isOn)
{
  if (isOn)
  {
    xTaskCreate(ntScanTask,
                "ntScanTask",
                4096,
                NULL,
                1,
                &ntScanTaskHandler);
  }
  else
  {
    this->WiFiScannerStop();
    WiFi.disconnect();
  }
}

void Network::WiFiScannerStop()
{
  if (ntScanTaskHandler != NULL)
  {
    vTaskDelete(ntScanTaskHandler);
    ntScanTaskHandler = NULL;
  }
}

void Network::WiFiConnector(void *param)
{
  this->WiFiScannerStop();

  String networkInfo = String((char *)param);
  int seperatorIdx = networkInfo.indexOf(WIFI_SSID_PW_DELIMITER);
  _ssid = networkInfo.substring(0, seperatorIdx);
  _pwd = networkInfo.substring(seperatorIdx + 2, networkInfo.length());
  Serial.println("Connecting to WiFi...");
  Serial.println("SSID: " + _ssid);
  Serial.println("PWD: " + _pwd);
  Serial.println("networkInfo: " + networkInfo);

  saveWiFiDetails(_ssid.c_str(), _pwd.c_str());
  xTaskCreate(ntBeginTask, "ntBeginTask", 4096, NULL, 1, &ntConnectTaskHandler);
}

void Network::saveWiFiDetails(const char *ssid, const char *password)
{
  Globals::get().preferences.begin("wifi", false);
  Globals::get().preferences.putString("ssid", ssid);
  Globals::get().preferences.putString("password", password);
  Globals::get().preferences.end();
}

void Network::loadWiFiDetails(String &ssid, String &password)
{
  Globals::get().preferences.begin("wifi", true);
  _ssid = Globals::get().preferences.getString("ssid", "");
  _pwd = Globals::get().preferences.getString("password", "");
  Globals::get().preferences.end();

  Globals::get().preferences.begin("settings", true);
  wifiSate = Globals::get().preferences.getBool("wifi_state", true);
  Globals::get().preferences.end();

  Serial.println("Loaded WiFi Details:");
  Serial.println("SSID: " + _ssid);
  Serial.println("PWD: " + _pwd);
  Serial.println("WiFi State: " + String(wifiSate ? "ON" : "OFF"));
}