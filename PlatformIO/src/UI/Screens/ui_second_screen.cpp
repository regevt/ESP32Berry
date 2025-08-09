/////////////////////////////////////////////////////////////////
/*
  Secondary screen construction
*/
/////////////////////////////////////////////////////////////////
#include "UI/ESP32Berry_Display.hpp"

void Display::ui_second()
{
  ui_Sub_Screen = lv_obj_create(NULL);

  lv_obj_clear_flag(ui_Sub_Screen, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(ui_Sub_Screen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Sub_Screen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
}
