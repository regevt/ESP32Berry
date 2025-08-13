/////////////////////////////////////////////////////////////////
/*
  New ESP32Berry Project, The base UI & ChatGPT Client
  For More Information: https://youtu.be/5K6rSw9j5iY
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#include "Apps/GPS/ESP32Berry_AppGPS.hpp"
#include <SoftwareSerial.h>
#include <Configurations/secrets.h>

static const int RXPin = 44;          // GPS TX → ESP32 RX
static const int TXPin = 43;          // GPS RX → ESP32 TX
static const uint32_t GPSBaud = 9600; // 9600

static TaskHandle_t gpsTaskHandler = NULL;

// SoftwareSerial ss(RXPin, TXPin);
HardwareSerial GPS(1); // Use UART1

static AppGPS *instance = NULL;

void GPStask(void *pvParameters)
{
  // instance->show_loading_popup(true);

  // while (GPS.available() > 0)
  //   if (gps.encode(GPS.read()))
  //     instance->draw_ui();

  // if (millis() > 5000 && gps.charsProcessed() < 10)
  // {
  //   Serial.println(F("No GPS detected: check wiring."));
  //   instance->_display->lv_port_sem_take();
  //   instance->show_loading_popup(false);
  //   instance->_display->lv_port_sem_give();
  //   while (true)
  //     ;
  // }
  // // The rest of the code here

  // instance->_display->lv_port_sem_take();
  // // instance->add_msg(false, result);
  // instance->show_loading_popup(false);
  // instance->_display->lv_port_sem_give();
  // vTaskDelete(NULL);

  instance->show_loading_popup(true);
  unsigned long start = millis();

  for (;;) // infinite loop
  {
    while (GPS.available() > 0)
    {
      if (gps.encode(GPS.read()))
      {
        instance->draw_ui();
      }
    }

    // Check if GPS is not responding
    if (millis() - start > 5000 && gps.charsProcessed() < 10)
    {
      Serial.println(F("No GPS detected: check wiring."));
      instance->_display->lv_port_sem_take();
      instance->show_loading_popup(false);
      instance->_display->lv_port_sem_give();
      vTaskDelete(NULL); // End task
    }

    vTaskDelay(10 / portTICK_PERIOD_MS); // Yield to other tasks
  }
}

void AppGPS::draw_ui()
{
  Serial.print("Satellites: ");
  Serial.print(gps.satellites.value());

  Serial.print(F("  Location: "));
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10)
      Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10)
      Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10)
      Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10)
      Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.println();
}

AppGPS::AppGPS(Display *display, System *system, Network *network, const char *title)
    : AppBase(display, system, network, title)
{
  instance = this;
  display_width = display->get_display_width();
  GPS.begin(GPSBaud, SERIAL_8N1, RXPin, TXPin); // (baud, config, RX, TX)

  // ss.begin(GPSBaud);

  xTaskCreate(GPStask, "gpsTask", 10000, NULL, 1, &gpsTaskHandler);
}

AppGPS::~AppGPS() {}

void AppGPS::close_app()
{
  _display->goback_main_screen();
  lv_obj_del(_bodyScreen);
  if (gpsTaskHandler != NULL)
  {
    vTaskDelete(gpsTaskHandler);
    gpsTaskHandler = NULL;
  }
  delete this;
}
