/////////////////////////////////////////////////////////////////
/*
  Popup and Loading UI helpers
*/
/////////////////////////////////////////////////////////////////
#include "UI/ESP32Berry_Display.hpp"

extern "C" void ui_event_callback_thunk(lv_event_t *e);

void Display::ui_prep_popup_box()
{

    ui_BasePopup = lv_obj_create(lv_scr_act());
    lv_obj_set_height(ui_BasePopup, 170);
    lv_obj_set_width(ui_BasePopup, lv_pct(93));
    lv_obj_set_align(ui_BasePopup, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_BasePopup, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_BasePopup, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_BasePopup, 196, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_BasePopupTitle = lv_label_create(ui_BasePopup);
    lv_obj_set_width(ui_BasePopupTitle, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_BasePopupTitle, LV_SIZE_CONTENT);
    lv_label_set_text(ui_BasePopupTitle, "");
    lv_obj_set_style_text_color(ui_BasePopupTitle, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_BasePopupTitle, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_BasePopupTitle, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_BasePopupMsg = lv_label_create(ui_BasePopup);
    lv_obj_set_width(ui_BasePopupMsg, 270);
    lv_obj_set_height(ui_BasePopupMsg, 110);
    lv_obj_set_x(ui_BasePopupMsg, 0);
    lv_obj_set_y(ui_BasePopupMsg, 30);
    lv_label_set_text(ui_BasePopupMsg, "");
    lv_obj_set_style_text_color(ui_BasePopupMsg, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_BasePopupMsg, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_BasePopupMsg, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_BasePopupCloseBtn = lv_btn_create(ui_BasePopup);
    lv_obj_set_width(ui_BasePopupCloseBtn, 50);
    lv_obj_set_height(ui_BasePopupCloseBtn, 34);
    lv_obj_set_align(ui_BasePopupCloseBtn, LV_ALIGN_BOTTOM_RIGHT);
    lv_obj_add_flag(ui_BasePopupCloseBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_BasePopupCloseBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_shadow_width(ui_BasePopupCloseBtn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui_BasePopupCloseBtn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_x(ui_BasePopupCloseBtn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(ui_BasePopupCloseBtn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(ui_BasePopupCloseBtn, ui_event_callback_thunk, LV_EVENT_CLICKED, NULL);

    lv_obj_t *ui_BaseOkBtnLabel = lv_label_create(ui_BasePopupCloseBtn);
    lv_obj_set_width(ui_BaseOkBtnLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_BaseOkBtnLabel, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_BaseOkBtnLabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_BaseOkBtnLabel, "OK");
}

void Display::ui_popup_open(String title, String msg)
{
    lv_obj_clear_flag(ui_BasePopup, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(ui_BasePopupTitle, title.c_str());
    lv_label_set_text(ui_BasePopupMsg, msg.c_str());
}

void Display::ui_prep_loading()
{
    ui_Loading = lv_obj_create(ui_Main_Screen);

    lv_obj_set_size(ui_Loading, 120, 140);
    lv_obj_t *loading_spinner = lv_spinner_create(ui_Loading, 1000, 60);
    lv_obj_set_size(loading_spinner, 80, 80);
    lv_obj_align(loading_spinner, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t *loading_label = lv_label_create(ui_Loading);
    lv_label_set_text(loading_label, "Loading...");
    lv_obj_align(loading_label, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_center(ui_Loading);
    this->show_loading_popup(false);
}

void Display::show_loading_popup(bool isOn)
{
    if (isOn)
    {
        lv_obj_clear_flag(ui_Loading, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_add_flag(ui_Loading, LV_OBJ_FLAG_HIDDEN);
    }
}
