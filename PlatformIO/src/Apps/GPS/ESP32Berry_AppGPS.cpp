/////////////////////////////////////////////////////////////////
/*
  New ESP32Berry Project, The base UI & ChatGPT Client
  For More Information: https://youtu.be/5K6rSw9j5iY
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#include "Apps/GPS/ESP32Berry_AppGPS.hpp"
#include <HTTPClient.h>

static TaskHandle_t gpsTaskHandler = NULL;

static AppGPS *instance = NULL;

double latitude = 0.0;
double longitude = 0.0;
uint32_t satellites = 0;

void GPStask(void *pvParameters)
{
  // instance->show_loading_popup(true);
  unsigned long start = millis();

  for (;;) // infinite loop
  {
    while (SerialGPS.available() > 0)
    {
      if (gps.encode(SerialGPS.read()))
      {
        if (gps.satellites.value() != satellites || latitude != gps.location.lat() || longitude != gps.location.lng())
        {
          latitude = gps.location.lat();
          longitude = gps.location.lng();
          satellites = gps.satellites.value();
          instance->_display->lv_port_sem_take();
          instance->draw_ui();
          instance->_display->lv_port_sem_give();
        }
      }
    }

    // Check if GPS is not responding
    if (millis() - start > 5000 && gps.charsProcessed() < 10)
    {
      Serial.println(F("No GPS detected: check wiring."));
      instance->_display->lv_port_sem_take();
      // instance->show_loading_popup(false);
      instance->_display->lv_port_sem_give();
      vTaskDelete(NULL); // End task
    }

    vTaskDelay(10 / portTICK_PERIOD_MS); // Yield to other tasks
  }
}

void AppGPS::draw_ui()
{
  String fullAddress = "Connecting to GPS...";
  Serial.print("Satellites: ");
  Serial.print(satellites);

  Serial.print(F("  Location: "));
  if (gps.location.isValid())
  {
    Serial.print(latitude, 6);
    Serial.print(F(","));
    Serial.print(longitude, 6);

    if (WiFi.status() == WL_CONNECTED && latitude != 0.0 && longitude != 0.0)
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
          JsonObject address = doc["address"];

          const char *house_number = address["house_number"] | "N/A";
          const char *road = address["road"] | "N/A";
          const char *city = address["city"] | address["town"] | address["village"] | "N/A";
          const char *country = address["country"] | "N/A";
          const char *postcode = address["postcode"] | "N/A";

          fullAddress = String(road) + " " + String(house_number) + ", " + String(city) + ", " + String(country) + " (" + String(postcode) + ")";

          Serial.print("Address:");
          Serial.print(fullAddress.c_str());
        }
        else
        {
          Serial.print("JSON parsing failed: ");
          Serial.println(error.c_str());
        }
      }
      else
      {
        Serial.print("Error on HTTP request");
      }

      http.end();
    }
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.println();

  static lv_coord_t row_declaration[] = {20, 20, 20, 20, LV_GRID_TEMPLATE_LAST};
  static lv_coord_t column_declaration[] = {88, 220, LV_GRID_TEMPLATE_LAST};
  lv_obj_t *grid_layout = lv_obj_create(ui_AppPanel);
  lv_obj_set_pos(grid_layout, 0, 0);
  lv_obj_set_size(grid_layout, 320, 110);
  lv_obj_set_style_layout(grid_layout, LV_LAYOUT_GRID, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_row_dsc_array(grid_layout, row_declaration, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_column_dsc_array(grid_layout, column_declaration, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_align(grid_layout, LV_ALIGN_TOP_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(grid_layout, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *latitude_lbl = lv_label_create(grid_layout);
  lv_obj_set_pos(latitude_lbl, 0, 0);
  lv_obj_set_size(latitude_lbl, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_style_grid_cell_column_pos(latitude_lbl, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_column_span(latitude_lbl, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_x_align(latitude_lbl, LV_GRID_ALIGN_START, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_row_pos(latitude_lbl, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_row_span(latitude_lbl, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_y_align(latitude_lbl, LV_GRID_ALIGN_START, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(latitude_lbl, "Latitude:");

  latitude_value = lv_label_create(grid_layout);
  lv_obj_set_pos(latitude_value, 0, 0);
  lv_obj_set_size(latitude_value, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_style_grid_cell_column_pos(latitude_value, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_column_span(latitude_value, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_x_align(latitude_value, LV_GRID_ALIGN_START, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_row_pos(latitude_value, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_row_span(latitude_value, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_y_align(latitude_value, LV_GRID_ALIGN_START, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(latitude_value, latitude != 0.0 ? String(latitude, 6).c_str() : "N/A");

  lv_obj_t *longitude_lbl = lv_label_create(grid_layout);
  lv_obj_set_pos(longitude_lbl, 0, 0);
  lv_obj_set_size(longitude_lbl, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_style_grid_cell_column_pos(longitude_lbl, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_column_span(longitude_lbl, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_x_align(longitude_lbl, LV_GRID_ALIGN_START, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_row_pos(longitude_lbl, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_row_span(longitude_lbl, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_y_align(longitude_lbl, LV_GRID_ALIGN_START, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(longitude_lbl, "Longitude:");

  longitude_value = lv_label_create(grid_layout);
  lv_obj_set_pos(longitude_value, 0, 0);
  lv_obj_set_size(longitude_value, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_style_grid_cell_column_pos(longitude_value, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_column_span(longitude_value, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_x_align(longitude_value, LV_GRID_ALIGN_START, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_row_pos(longitude_value, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_row_span(longitude_value, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_y_align(longitude_value, LV_GRID_ALIGN_START, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(longitude_value, longitude != 0.0 ? String(longitude, 6).c_str() : "N/A");

  lv_obj_t *satellites_lbl = lv_label_create(grid_layout);
  lv_obj_set_pos(satellites_lbl, 0, 0);
  lv_obj_set_size(satellites_lbl, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_style_grid_cell_column_pos(satellites_lbl, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_column_span(satellites_lbl, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_x_align(satellites_lbl, LV_GRID_ALIGN_START, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_row_pos(satellites_lbl, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_row_span(satellites_lbl, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_y_align(satellites_lbl, LV_GRID_ALIGN_START, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(satellites_lbl, "Satellites:");

  satellites_value = lv_label_create(grid_layout);
  lv_obj_set_pos(satellites_value, 0, 0);
  lv_obj_set_size(satellites_value, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_style_grid_cell_column_pos(satellites_value, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_column_span(satellites_value, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_x_align(satellites_value, LV_GRID_ALIGN_START, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_row_pos(satellites_value, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_row_span(satellites_value, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_y_align(satellites_value, LV_GRID_ALIGN_START, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(satellites_value, satellites > 0 ? String(satellites).c_str() : "N/A");

  lv_obj_t *address_lbl = lv_label_create(grid_layout);
  lv_obj_set_pos(address_lbl, 0, 0);
  lv_obj_set_size(address_lbl, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_style_grid_cell_column_pos(address_lbl, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_column_span(address_lbl, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_x_align(address_lbl, LV_GRID_ALIGN_START, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_row_pos(address_lbl, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_row_span(address_lbl, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_y_align(address_lbl, LV_GRID_ALIGN_START, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(address_lbl, "Address:");

  address_value = lv_label_create(grid_layout);
  lv_obj_set_pos(address_value, 0, 0);
  lv_obj_set_size(address_value, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_style_grid_cell_column_pos(address_value, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_column_span(address_value, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_x_align(address_value, LV_GRID_ALIGN_START, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_row_pos(address_value, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_row_span(address_value, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_grid_cell_y_align(address_value, LV_GRID_ALIGN_START, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(address_value, fullAddress.c_str());
}

AppGPS::AppGPS(Display *display, System *system, Network *network, const char *title)
    : AppBase(display, system, network, title)
{
  instance = this;
  display_width = display->get_display_width();
  SerialGPS.begin(GPSBaud, SERIAL_8N1, BOARD_GPS_RX_PIN, BOARD_GPS_TX_PIN);
  instance->_display->lv_port_sem_take();
  instance->draw_ui();
  instance->_display->lv_port_sem_give();
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
