/////////////////////////////////////////////////////////////////
/*
  New ESP32Berry Project, The base UI & ChatGPT Client
  For More Information: https://youtu.be/5K6rSw9j5iY
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#include "Apps/GPS/ESP32Berry_AppGPS.hpp"
#include <HTTPClient.h>
#include <Utils/Globals.h>
#include <Utils/EventManager/EventManager.h>

static TaskHandle_t gpsTaskHandler = NULL;

static AppGPS *instance = NULL;

double latitude = 0.0;
double longitude = 0.0;
uint32_t satellites = 0;

void GPStask(void *pvParameters)
{
  unsigned long start = millis();
  static unsigned long last_ui_update = 0; // throttle UI updates

  for (;;) // infinite loop
  {
    while (SerialGPS.available() > 0)
    {
      if (gps.encode(SerialGPS.read()))
      {
        bool changed = (gps.satellites.value() != satellites || latitude != gps.location.lat() || longitude != gps.location.lng());
        unsigned long now = millis();
        bool time_due = (now - last_ui_update) >= 10000UL; // 10 seconds

        if (changed)
        {
          // Always keep internal state up-to-date
          latitude = gps.location.lat();
          longitude = gps.location.lng();
          satellites = gps.satellites.value();
        }

        if (time_due)
        {
          String address = instance->get_current_address(); // default to last known
          // Only reverse geocode when we have a new position (avoid spamming)
          if (changed && WiFi.status() == WL_CONNECTED && gps.location.isValid() && latitude != 0.0 && longitude != 0.0)
          {
            HTTPClient http;
            String url = "https://nominatim.openstreetmap.org/reverse?lat=" + String(latitude, 6) + "&lon=" + String(longitude, 6) + "&format=json";
            http.begin(url);
            int httpResponseCode = http.GET();
            if (httpResponseCode > 0)
            {
              String payload = http.getString();
              JsonDocument doc;
              DeserializationError error = deserializeJson(doc, payload);
              if (!error)
              {
                JsonObject addr = doc["address"];
                const char *house_number = addr["house_number"] | "";
                const char *road = addr["road"] | "";
                const char *city = addr["city"] | addr["town"] | addr["village"] | "";
                const char *country = addr["country"] | "";
                const char *postcode = addr["postcode"] | "";
                address = String(road) + (strlen(house_number) ? String(" ") + house_number : "") + ", " + String(city) + ", " + String(country) + (strlen(postcode) ? String(" (") + postcode + ")" : "");
              }
            }
            http.end();
          }

          // Update UI under LVGL lock. Create UI once and only set labels.
          GlobalEventBus.emit(Events::PORT_SEM_TAKE, "");
          instance->ensure_ui_created();
          instance->update_ui(latitude, longitude, satellites, address);
          GlobalEventBus.emit(Events::PORT_SEM_GIVE, "");
          last_ui_update = now;
        }
      }
    }

    // Check if GPS is not responding
    if (millis() - start > 5000 && gps.charsProcessed() < 10)
    {
      Serial.println(F("No GPS detected: check wiring."));
      vTaskDelete(NULL); // End task
    }

    vTaskDelay(20 / portTICK_PERIOD_MS); // Yield to other tasks
  }
}

void AppGPS::draw_ui()
{
  ensure_ui_created();
  update_ui(latitude, longitude, satellites, current_address);
}

void AppGPS::ensure_ui_created()
{
  if (grid_layout != nullptr)
    return;
  static lv_coord_t row_declaration[] = {20, 20, 20, 40, LV_GRID_TEMPLATE_LAST};
  static lv_coord_t column_declaration[] = {88, 220, LV_GRID_TEMPLATE_LAST};
  grid_layout = lv_obj_create(ui_AppPanel);
  lv_obj_set_pos(grid_layout, 0, 0);
  lv_obj_set_size(grid_layout, 320, 140);
  // Enable grid layout on the container
  lv_obj_set_layout(grid_layout, LV_LAYOUT_GRID);
  // Disable scrolling on the container
  lv_obj_clear_flag(grid_layout, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scrollbar_mode(grid_layout, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_style_grid_row_dsc_array(grid_layout, row_declaration, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_column_dsc_array(grid_layout, column_declaration, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_align(grid_layout, LV_ALIGN_TOP_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(grid_layout, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *latitude_lbl = lv_label_create(grid_layout);
  lv_obj_set_grid_cell(latitude_lbl, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 0, 1);
  lv_label_set_text(latitude_lbl, "Latitude:");

  latitude_value = lv_label_create(grid_layout);
  lv_obj_set_grid_cell(latitude_value, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 0, 1);
  lv_label_set_text(latitude_value, "N/A");

  lv_obj_t *longitude_lbl = lv_label_create(grid_layout);
  lv_obj_set_grid_cell(longitude_lbl, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 1, 1);
  lv_label_set_text(longitude_lbl, "Longitude:");

  longitude_value = lv_label_create(grid_layout);
  lv_obj_set_grid_cell(longitude_value, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 1, 1);
  lv_label_set_text(longitude_value, "N/A");

  lv_obj_t *satellites_lbl = lv_label_create(grid_layout);
  lv_obj_set_grid_cell(satellites_lbl, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 2, 1);
  lv_label_set_text(satellites_lbl, "Satellites:");

  satellites_value = lv_label_create(grid_layout);
  lv_obj_set_grid_cell(satellites_value, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 2, 1);
  lv_label_set_text(satellites_value, "N/A");

  lv_obj_t *address_lbl = lv_label_create(grid_layout);
  lv_obj_set_grid_cell(address_lbl, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 3, 1);
  lv_label_set_text(address_lbl, "Address:");

  address_value = lv_label_create(grid_layout);
  lv_obj_set_grid_cell(address_value, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 3, 1);
  // Make the address scroll like a billboard
  lv_obj_set_width(address_value, 220);
  lv_label_set_long_mode(address_value, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_label_set_text(address_value, current_address.c_str());
}

void AppGPS::update_ui(double lat, double lon, uint32_t sats, const String &address)
{
  // Update text values; avoid using temporary String.c_str() that goes out of scope mid-call
  static char lat_buf[24];
  static char lon_buf[24];
  static char sat_buf[12];
  snprintf(lat_buf, sizeof(lat_buf), "%s", (lat != 0.0) ? String(lat, 6).c_str() : "N/A");
  snprintf(lon_buf, sizeof(lon_buf), "%s", (lon != 0.0) ? String(lon, 6).c_str() : "N/A");
  snprintf(sat_buf, sizeof(sat_buf), "%s", (sats > 0) ? String(sats).c_str() : "N/A");
  if (latitude_value)
    lv_label_set_text(latitude_value, lat_buf);
  if (longitude_value)
    lv_label_set_text(longitude_value, lon_buf);
  if (satellites_value)
    lv_label_set_text(satellites_value, sat_buf);
  current_address = address.length() ? address : current_address;
  if (address_value)
    lv_label_set_text(address_value, current_address.c_str());
  // Ensure marquee stays active if created earlier
  if (address_value)
  {
    lv_obj_set_width(address_value, 220);
    lv_label_set_long_mode(address_value, LV_LABEL_LONG_SCROLL_CIRCULAR);
  }
}

AppGPS::AppGPS(lv_obj_t *screen, const char *title)
    : AppBase(screen, title)
{
  instance = this;
  display_width = Globals::get().screen_width;
  SerialGPS.begin(GPSBaud, SERIAL_8N1, BOARD_GPS_RX_PIN, BOARD_GPS_TX_PIN);
  GlobalEventBus.emit(Events::PORT_SEM_TAKE, "");
  ensure_ui_created();
  update_ui(latitude, longitude, satellites, current_address);
  GlobalEventBus.emit(Events::PORT_SEM_GIVE, "");
  xTaskCreate(GPStask, "gpsTask", 10000, NULL, 1, &gpsTaskHandler);
}

AppGPS::~AppGPS() {}

void AppGPS::close_app()
{
  GlobalEventBus.emit(Events::GO_BACK_MAIN_SCREEN, "");
  lv_obj_del(_bodyScreen);
  if (gpsTaskHandler != NULL)
  {
    vTaskDelete(gpsTaskHandler);
    gpsTaskHandler = NULL;
  }
  delete this;
}
