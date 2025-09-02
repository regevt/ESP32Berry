#include "Apps/FileBrowser/ESP32Berry_AppFileBrowser.hpp"
#include <Configurations/secrets.h>
#include "Utils/BusLock.hpp"

static AppFileBrowser *instance = NULL;

LV_IMG_DECLARE(folder_icon);

extern "C" void fb_folder_click_event_cb_thunk(lv_event_t *e)
{
  const char *folderNameCStr = static_cast<const char *>(lv_event_get_user_data(e));
  std::string folderName(folderNameCStr ? folderNameCStr : "/");
  instance->history.push_back(instance->currentDir);
  instance->currentDir = folderName;
  instance->printFiles(folderName);
}

extern "C" void fb_back_click_event_cb_thunk(lv_event_t *e)
{
  string prev = instance->history.back();
  instance->history.pop_back();
  instance->printFiles(prev.length() > 0 ? prev : "/");
}

AppFileBrowser::AppFileBrowser(Display *display, System *system, Network *network, const char *title)
    : AppBase(display, system, network, title)
{
  instance = this;
  display_width = display->get_display_width();
  client.setInsecure();
  this->initFileBrowser();
}

AppFileBrowser::~AppFileBrowser() {}

void fileBrowserTsk(void *pvParameters)
{
  if (!instance->_system->isSDCard)
  {
    instance->_system->initSDCard();
    Serial.println("SD Card not initialized or not available.!!!");
  }
  else
  {
    instance->history.clear();
    instance->printFiles("/");
    instance->currentDir = "/";
    Serial.println("File Browser Task Completed");
  }

  instance->_display->lv_port_sem_take();
  // instance->show_loading_popup(false);
  instance->_display->lv_port_sem_give();

  vTaskDelete(NULL);
}

void AppFileBrowser::initFileBrowser()
{

  xTaskCreate(fileBrowserTsk, "fileBrowserTsk", 10000, NULL, 1, NULL);
}

void AppFileBrowser::printFiles(string rootDir)
{
  std::vector<fs::File> dirs = instance->_system->listDir(rootDir.c_str()); // List root directory contents

  // Create a scrollable container to hold the directory entries
  instance->_display->lv_port_sem_take();
  lv_obj_t *fileListCont = lv_obj_create(instance->bodyScreen());
  lv_obj_set_x(fileListCont, -12);
  lv_obj_set_y(fileListCont, 14);
  lv_obj_set_size(fileListCont,
                  instance->_display->get_display_width(),
                  instance->_display->get_display_height() - 70);
  lv_obj_set_align(fileListCont, LV_ALIGN_TOP_LEFT);
  lv_obj_set_style_pad_all(fileListCont, 0, 0);
  lv_obj_set_scroll_dir(fileListCont, LV_DIR_VER);                   // enable vertical scrolling
  lv_obj_set_scrollbar_mode(fileListCont, LV_SCROLLBAR_MODE_ACTIVE); // always show scrollbar when scrollable
  lv_obj_set_style_border_width(fileListCont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  int counter = 0;
  for (auto &dir : dirs)
  {
    if (dir.name()[0] == '.')
    {
      continue; // Skip hidden files and directories
    }

    if (dir.isDirectory())
    {

      lv_obj_t *folderIcon = lv_img_create(fileListCont);
      const lv_img_dsc_t *dsc = &folder_icon;
      uint16_t base = LV_MAX(dsc->header.w, dsc->header.h);
      uint16_t zoom = (uint16_t)((18u * 256u) / base);
      lv_image_set_scale(folderIcon, zoom);
      lv_obj_set_size(folderIcon, (dsc->header.w * zoom) / 256, (dsc->header.h * zoom) / 256);
      lv_image_set_src(folderIcon, &folder_icon);
      lv_obj_set_width(folderIcon, 18);
      lv_obj_set_height(folderIcon, 18);
      lv_obj_set_x(folderIcon, 10);
      lv_obj_set_y(folderIcon, counter * 20 + 10);
      lv_obj_set_align(folderIcon, LV_ALIGN_TOP_LEFT);
    }

    lv_obj_t *fileLbl = lv_label_create(fileListCont);
    lv_obj_set_width(fileLbl, instance->_display->get_display_width() - 20);
    lv_obj_set_height(fileLbl, LV_SIZE_CONTENT);
    lv_obj_set_x(fileLbl, 38);
    lv_obj_set_y(fileLbl, counter * 24 + 10);
    lv_obj_set_align(fileLbl, LV_ALIGN_TOP_LEFT);
    lv_label_set_text(fileLbl, dir.name());
    lv_obj_set_style_text_color(fileLbl, lv_color_hex(0x3D3D3D), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(fileLbl, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(fileLbl, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    if (dir.isDirectory())
    {
      lv_obj_add_event_cb(fileLbl, fb_folder_click_event_cb_thunk, LV_EVENT_CLICKED, (void *)dir.path());
      lv_obj_add_flag(fileLbl, LV_OBJ_FLAG_CLICKABLE);
    }
    else
    {
      // For files, you can add a click event to handle file opening or other actions
      // lv_obj_add_event_cb(fileLbl, file_click_event_cb_thunk, LV_EVENT_CLICKED, (void *)dir.name());
    }

    counter++;
  }

  // back button
  lv_obj_t *backBtn = lv_button_create(instance->bodyScreen());
  lv_obj_set_size(backBtn, 70, 30);
  lv_obj_set_x(backBtn, 0);
  lv_obj_set_y(backBtn, instance->_display->get_display_height() - 50);
  lv_obj_set_align(backBtn, LV_ALIGN_TOP_LEFT);
  lv_obj_set_style_bg_color(backBtn, lv_color_hex(0xE0E0E0), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(backBtn, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(backBtn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(backBtn, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_t *backBtnLabel = lv_label_create(backBtn);
  lv_label_set_text(backBtnLabel, "Back");
  lv_obj_set_style_text_color(backBtnLabel, lv_color_hex(0x3D3D3D), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(backBtnLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(backBtnLabel, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_event_cb(backBtn, fb_back_click_event_cb_thunk, LV_EVENT_CLICKED, (void *)rootDir.c_str());
  lv_obj_set_style_bg_color(backBtn, lv_color_hex(0xEEEEEE), LV_PART_MAIN | LV_STATE_DISABLED);
  lv_obj_set_style_bg_opa(backBtn, 255, LV_PART_MAIN | LV_STATE_DISABLED);
  lv_obj_set_style_border_width(backBtn, 0, LV_PART_MAIN | LV_STATE_DISABLED);

  if (instance->currentDir.c_str() == "/" || instance->history.size() < 1)
  {
    lv_obj_add_state(backBtn, LV_STATE_DISABLED);
    lv_obj_remove_flag(backBtn, LV_OBJ_FLAG_CLICKABLE);
  }
  else
  {
    lv_obj_remove_state(backBtn, LV_STATE_DISABLED);
    lv_obj_add_flag(backBtn, LV_OBJ_FLAG_CLICKABLE);
  }

  instance->_display->lv_port_sem_give();
}

void AppFileBrowser::close_app()
{
  _display->goback_main_screen();
  lv_obj_del(_bodyScreen);
  delete this;
}
