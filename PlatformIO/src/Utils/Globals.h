#pragma once
#include <Arduino.h>
#include <Preferences.h>

namespace Events
{
    static const String VOLUME_CHANGED = "volume_changed";
    static const String BRIGHTNESS_CHANGED = "brightness_changed";
    static const String SET_WIFI = "set_wifi";
    static const String BATTERY_VALUE_CHANGED = "battery_value_changed";
    static const String TOGGLE_CURSOR = "toggle_cursor";
    static const String UI_ACTIVITY = "ui_activity";
    static const String SCREEN_TIMEOUT_CHANGED = "screen_timeout_changed";
    static const String PORT_SEM_TAKE = "port_sem_take";
    static const String PORT_SEM_GIVE = "port_sem_give";
}

struct Globals
{
    bool cursor_panel_active = false; // Only initialized once!
    u_int32_t screen_width = 320;
    u_int32_t screen_height = 240;
    Preferences preferences;

    // --- Singleton Accessor ---
    static Globals &get()
    {
        static Globals instance;
        return instance;
    }

    // Prevent copying
    Globals(const Globals &) = delete;
    Globals &operator=(const Globals &) = delete;

private:
    Globals() {} // private constructor
};