// Placeholder for a MicroPython native module that can wrap selected C++
// Build integration will be added via lv_micropython/user_modules.

#include "py/obj.h"

STATIC mp_obj_t esp32berry_hello(void) {
    return mp_obj_new_str("hello from native", 17);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(esp32berry_hello_obj, esp32berry_hello);

STATIC const mp_rom_map_elem_t esp32berry_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_esp32berry) },
    { MP_ROM_QSTR(MP_QSTR_hello), MP_ROM_PTR(&esp32berry_hello_obj) },
};
STATIC MP_DEFINE_CONST_DICT(esp32berry_module_globals, esp32berry_module_globals_table);

const mp_obj_module_t esp32berry_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&esp32berry_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_esp32berry, esp32berry_user_cmodule);
