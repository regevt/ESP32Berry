#pragma once
#include "Configurations/ESP32Berry_Config.hpp"
#include "Apps/ESP32Berry_AppBase.hpp"
#include <Arduino.h>
#include <vector>
#include <string>

class AppMp3 : public AppBase
{
private:
  void draw_ui();
  void build_controls();
  void scan_playlist();
  void start_play(size_t index);
  static void timer_update_cb(lv_timer_t *t);
  void update_progress_ui();
  void handle_seek_commit(int32_t value);
  static bool has_ext_ci(const char *name, const char *ext);

  // UI elements
  lv_obj_t *lbl_track = nullptr;
  lv_obj_t *btn_prev = nullptr;
  lv_obj_t *btn_play = nullptr;
  lv_obj_t *btn_next = nullptr;
  lv_obj_t *slider_seek = nullptr;
  lv_obj_t *lbl_time_left = nullptr;  // current time
  lv_obj_t *lbl_time_right = nullptr; // total time
  lv_timer_t *progress_timer = nullptr;

  // State
  std::vector<std::string> playlist; // absolute paths
  size_t current_index = 0;
  bool user_dragging = false;
  bool playing = false;
  bool last_running = false;

  // Event handlers
  static void btn_prev_cb_thunk(lv_event_t *e);
  static void btn_play_cb_thunk(lv_event_t *e);
  static void btn_next_cb_thunk(lv_event_t *e);
  static void slider_event_cb_thunk(lv_event_t *e);
  void on_prev();
  void on_play_pause();
  void on_next();

public:
  AppMp3(System *system, lv_obj_t *screen, const char *title);
  ~AppMp3();

  System *_system;
  void close_app();
};
