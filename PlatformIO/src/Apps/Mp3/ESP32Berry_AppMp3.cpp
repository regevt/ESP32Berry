#include "Apps/Mp3/ESP32Berry_AppMp3.hpp"
#include "Utils/BusLock.hpp"

static AppMp3 *instance = NULL;

AppMp3::AppMp3(Display *display, System *system, Network *network, const char *title)
	: AppBase(display, system, network, title) {
  instance = this;
  this->draw_ui();
}

AppMp3::~AppMp3() {}

void AppMp3::draw_ui() {
  build_controls();
  scan_playlist();

  // Start timer to update progress UI every 200ms
  progress_timer = lv_timer_create(timer_update_cb, 200, this);
}

void AppMp3::close_app() {
  if (progress_timer) {
    lv_timer_del(progress_timer);
    progress_timer = nullptr;
  }
  _display->goback_main_screen();
  lv_obj_del(_bodyScreen);
  delete this;
}

// Helpers
bool AppMp3::has_ext_ci(const char *name, const char *ext) {
  if (!name || !ext) return false;
  size_t ln = strlen(name);
  size_t le = strlen(ext);
  if (ln < le) return false;
  const char *p = name + (ln - le);
  for (size_t i = 0; i < le; ++i) {
    char c1 = p[i];
    char c2 = ext[i];
    if (c1 >= 'A' && c1 <= 'Z') c1 = c1 - 'A' + 'a';
    if (c2 >= 'A' && c2 <= 'Z') c2 = c2 - 'A' + 'a';
    if (c1 != c2) return false;
  }
  return true;
}

void AppMp3::build_controls() {
  // Title label for current track
  lbl_track = lv_label_create(ui_AppPanel);
  lv_obj_set_width(lbl_track, _display->get_display_width() - 20);
  lv_obj_set_style_text_align(lbl_track, LV_TEXT_ALIGN_CENTER, 0);
  lv_label_set_text(lbl_track, "No track loaded");
  lv_obj_align(lbl_track, LV_ALIGN_TOP_MID, 0, 6);

  // Seek slider
  slider_seek = lv_slider_create(ui_AppPanel);
  lv_obj_set_width(slider_seek, _display->get_display_width() - 140);
  lv_slider_set_range(slider_seek, 0, 1000); // we'll map to duration seconds
  lv_obj_align(slider_seek, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_event_cb(slider_seek, slider_event_cb_thunk, LV_EVENT_ALL, this);

  // Time labels (left current, right total)
  lbl_time_left = lv_label_create(ui_AppPanel);
  lv_label_set_text(lbl_time_left, "0:00");
  lv_obj_align_to(lbl_time_left, slider_seek, LV_ALIGN_OUT_LEFT_MID, -20, 0);

  lbl_time_right = lv_label_create(ui_AppPanel);
  lv_label_set_text(lbl_time_right, "0:00");
  lv_obj_align_to(lbl_time_right, slider_seek, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

  // Controls: Prev | Play/Pause | Next
  btn_prev = lv_button_create(ui_AppPanel);
  lv_obj_set_size(btn_prev, 70, 36);
  lv_obj_align(btn_prev, LV_ALIGN_BOTTOM_LEFT, 20, -12);
  lv_obj_add_event_cb(btn_prev, btn_prev_cb_thunk, LV_EVENT_CLICKED, this);
  lv_obj_t *lbl_prev = lv_label_create(btn_prev);
  lv_label_set_text(lbl_prev, "Prev");
  lv_obj_center(lbl_prev);

  btn_play = lv_button_create(ui_AppPanel);
  lv_obj_set_size(btn_play, 90, 36);
  lv_obj_align(btn_play, LV_ALIGN_BOTTOM_MID, 0, -12);
  lv_obj_add_event_cb(btn_play, btn_play_cb_thunk, LV_EVENT_CLICKED, this);
  lv_obj_t *lbl_play = lv_label_create(btn_play);
  lv_label_set_text(lbl_play, "Play");
  lv_obj_center(lbl_play);

  btn_next = lv_button_create(ui_AppPanel);
  lv_obj_set_size(btn_next, 70, 36);
  lv_obj_align(btn_next, LV_ALIGN_BOTTOM_RIGHT, -20, -12);
  lv_obj_add_event_cb(btn_next, btn_next_cb_thunk, LV_EVENT_CLICKED, this);
  lv_obj_t *lbl_next = lv_label_create(btn_next);
  lv_label_set_text(lbl_next, "Next");
  lv_obj_center(lbl_next);
}

void AppMp3::scan_playlist() {
  playlist.clear();
  if (!_system->isSDCard) {
    _system->initSDCard();
  }
  if (!_system->isSDCard) {
    lv_label_set_text(lbl_track, "SD not available");
    return;
  }

  // Shallow scan of root and /music for mp3 files
  auto collect = [&](const char *dir){
    std::vector<fs::File> entries = _system->listDir(dir);
    for (auto &f : entries) {
      if (!f.isDirectory()) {
        const char *name = f.name();
        if (has_ext_ci(name, ".mp3") || has_ext_ci(name, ".wav") || has_ext_ci(name, ".aac") || has_ext_ci(name, ".flac")) {
          // Prefer absolute paths; File::path() if available, else construct
          const char *p = f.path();
          if (p && strlen(p) > 0) {
            playlist.emplace_back(p);
          } else {
            std::string full = std::string(dir);
            if (full.length() > 1 && full.back() != '/') full.push_back('/');
            full += name;
            playlist.emplace_back(full);
          }
        }
      }
    }
  };

  collect("/");
  if (_system->createDir("/music")) {
    collect("/music");
  }

  if (playlist.empty()) {
    lv_label_set_text(lbl_track, "No audio files found");
  } else {
    current_index = 0;
    // start_play(current_index);
  }
}

void AppMp3::start_play(size_t index) {
  if (playlist.empty() || index >= playlist.size()) return;
  current_index = index;
  const char *path = playlist[current_index].c_str();
  lv_label_set_text(lbl_track, path);

  // Stop any current
  if (_system->audio && _system->audio->isRunning()) {
    _system->audio->stopSong();
    vTaskDelay(2);
  }

  // Kick off playback task via System API
  _system->play_audio(path);
  playing = true;

  // Update play button label
  if (btn_play) {
    lv_obj_t *child = lv_obj_get_child(btn_play, 0);
    if (child) lv_label_set_text(child, "Pause");
  }
}

// Time formatting helper
static void fmt_time(char *out, size_t n, uint32_t sec) {
  uint32_t m = sec / 60U;
  uint32_t s = sec % 60U;
  snprintf(out, n, "%lu:%02lu", (unsigned long)m, (unsigned long)s);
}

void AppMp3::update_progress_ui() {
  if (!_system || !_system->audio) return;
  // Don't fight the user while dragging
  if (user_dragging) return;

  bool running = _system->audio->isRunning();
  if (last_running && !running && !user_dragging) {
    // Ended: auto-advance
    on_next();
  }
  last_running = running;

  uint32_t cur = _system->audio->getAudioCurrentTime();
  uint32_t tot = _system->audio->getAudioFileDuration();

  char l[12];
  char r[12];
  fmt_time(l, sizeof(l), cur);
  fmt_time(r, sizeof(r), tot);

  if (lbl_time_left) lv_label_set_text(lbl_time_left, l);
  if (lbl_time_right) lv_label_set_text(lbl_time_right, r);

  if (tot > 0 && slider_seek) {
    // Map seconds to 0..1000
    uint32_t pos = (uint64_t)cur * 1000ULL / (uint64_t)tot;
    if (pos > 1000) pos = 1000;
    lv_slider_set_value(slider_seek, (int32_t)pos, LV_ANIM_OFF);
  }

  // Update play button label based on running state
  if (btn_play) {
    lv_obj_t *child = lv_obj_get_child(btn_play, 0);
    if (child) lv_label_set_text(child, running ? "Pause" : "Play");
  }
}

void AppMp3::handle_seek_commit(int32_t value) {
  if (!_system || !_system->audio) return;
  uint32_t tot = _system->audio->getAudioFileDuration();
  if (tot == 0) return;
  // Map 0..1000 to seconds
  uint32_t sec = (uint64_t)value * (uint64_t)tot / 1000ULL;
  // Avoid holding SPI while LV callbacks run; lock around Audio calls
  if (spi_bus_try_lock(pdMS_TO_TICKS(50))) {
    _system->audio->setAudioPlayPosition(sec);
    spi_bus_unlock();
  }
}

// LVGL callbacks (thunks)
void AppMp3::timer_update_cb(lv_timer_t *t) {
  AppMp3 *self = static_cast<AppMp3*>(lv_timer_get_user_data(t));
  if (!self) return;
  self->update_progress_ui();
}

void AppMp3::btn_prev_cb_thunk(lv_event_t *e) {
  AppMp3 *self = static_cast<AppMp3*>(lv_event_get_user_data(e));
  if (!self) return;
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) self->on_prev();
}

void AppMp3::btn_play_cb_thunk(lv_event_t *e) {
  AppMp3 *self = static_cast<AppMp3*>(lv_event_get_user_data(e));
  if (!self) return;
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) self->on_play_pause();
}

void AppMp3::btn_next_cb_thunk(lv_event_t *e) {
  AppMp3 *self = static_cast<AppMp3*>(lv_event_get_user_data(e));
  if (!self) return;
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) self->on_next();
}

void AppMp3::slider_event_cb_thunk(lv_event_t *e) {
  AppMp3 *self = static_cast<AppMp3*>(lv_event_get_user_data(e));
  if (!self) return;
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_PRESSED) {
    self->user_dragging = true;
  } else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
    self->user_dragging = false;
    int32_t v = lv_slider_get_value(self->slider_seek);
    self->handle_seek_commit(v);
  } else if (code == LV_EVENT_VALUE_CHANGED) {
    // Live preview current time label while dragging
    if (self->user_dragging && self->_system && self->_system->audio) {
      uint32_t tot = self->_system->audio->getAudioFileDuration();
      if (tot > 0) {
        int32_t v = lv_slider_get_value(self->slider_seek);
        uint32_t sec = (uint64_t)v * (uint64_t)tot / 1000ULL;
        char l[12]; fmt_time(l, sizeof(l), sec);
        lv_label_set_text(self->lbl_time_left, l);
      }
    }
  }
}

// Button handlers
void AppMp3::on_prev() {
  if (playlist.empty()) return;
  size_t idx = (current_index == 0) ? (playlist.size() - 1) : (current_index - 1);
  start_play(idx);
}

void AppMp3::on_play_pause() {
  if (!_system || !_system->audio) return;
  if (playlist.empty()) return;
  // If not running, (re)start current track
  if (!_system->audio->isRunning()) {
    start_play(current_index);
    return;
  }
  bool now = _system->audio->pauseResume();
  playing = now; // library returns true when playing?
  if (btn_play) {
    lv_obj_t *child = lv_obj_get_child(btn_play, 0);
    if (child) lv_label_set_text(child, now ? "Pause" : "Play");
  }
}

void AppMp3::on_next() {
  if (playlist.empty()) return;
  size_t idx = (current_index + 1) % playlist.size();
  start_play(idx);
}
