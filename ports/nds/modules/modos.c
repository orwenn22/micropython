#include "py/builtin.h"
#include "py/runtime.h"

#include <dirent.h>
#include <stdio.h>
#include <unistd.h>


//////////////////////////
// chdir(path : str)
STATIC mp_obj_t py_os_chdir(mp_obj_t path) {
    const char* c_path = mp_obj_str_get_str(path);
    return mp_obj_new_int(chdir(c_path));
}
MP_DEFINE_CONST_FUN_OBJ_1(os_chdir_obj, py_os_chdir);


//////////////////////////
// getcwd()
STATIC mp_obj_t py_os_getcwd(void) {
    char cwd[256];
    getcwd(cwd, (size_t)256);
    return mp_obj_new_str(cwd, strlen(cwd));
}
MP_DEFINE_CONST_FUN_OBJ_0(os_getcwd_obj, py_os_getcwd);


//////////////////////////
// listdir() -> List[str]
STATIC mp_obj_t py_os_listdir(void) {
    mp_obj_t l = mp_obj_new_list(0, NULL);  //initialise empty list.

    char cwd[256];
    getcwd(cwd, (size_t)256);

    //DIR* targetdir = opendir(g_currentworkingdir);
    DIR* targetdir = opendir(cwd);
    if(targetdir != NULL) {
        while(true) {
            struct dirent* pent = readdir(targetdir);
            if(pent == NULL) break;

            mp_obj_t strfilename = mp_obj_new_str(pent->d_name, strlen(pent->d_name));
            mp_obj_list_append(l, strfilename);
        }
    }
    else {
        printf("Directory doesn't exist\n");
    }
    closedir(targetdir);

    //mp_obj_list_append(l, mp_obj_new_int(42));
    return l;
}
MP_DEFINE_CONST_FUN_OBJ_0(os_listdir_obj, py_os_listdir);

//////////////////////////////////////////////////////////////
//// Define the module itself


//List all the things in the os/os module
STATIC const mp_rom_map_elem_t mp_module_os_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__)         , MP_ROM_QSTR(MP_QSTR_os) },               //module name "os"/"os"
    { MP_ROM_QSTR(MP_QSTR_chdir)            , MP_ROM_PTR(&os_chdir_obj)},
    { MP_ROM_QSTR(MP_QSTR_getcwd)           , MP_ROM_PTR(&os_getcwd_obj)},
    { MP_ROM_QSTR(MP_QSTR_listdir)          , MP_ROM_PTR(&os_listdir_obj)},

};
STATIC MP_DEFINE_CONST_DICT(mp_module_os_globals, mp_module_os_globals_table);


//Module object
const mp_obj_module_t mp_module_os = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mp_module_os_globals,
};

MP_REGISTER_MODULE(MP_QSTR_os, mp_module_os);