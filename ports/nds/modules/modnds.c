#include "py/builtin.h"
#include "py/runtime.h"
#include "py/stackctrl.h"
#include "py/mperrno.h"

#include <string.h>

#include <fatfs.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>


#include "../nitrofile.h"


const mp_obj_t myconst = MP_ROM_INT(42);

//////////////////////////
//printstackusage()
STATIC mp_obj_t py_nds_printstackusage(void) {
    printf("Stack usage : %u\n", mp_stack_usage());
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(nds_printstackusage_obj, py_nds_printstackusage);


//////////////////////////
// nitrofsinit() -> Bool
STATIC mp_obj_t py_nds_nitrofsinit(void) {
    bool r = nitroFSInit(NULL);
    if(r) {
        chdir("nitro:/root");
    }
    return mp_obj_new_bool(r);
}
MP_DEFINE_CONST_FUN_OBJ_0(nds_nitrofsinit_obj, py_nds_nitrofsinit);


///////////////////////////
// open(path : str, mode : str) -> NitroFile
//TODO : move this somewhere else ?
mp_obj_t py_nds_open(mp_obj_t path, mp_obj_t mode) {
    const char* c_path = mp_obj_str_get_str(path);
    const char* c_mode = mp_obj_str_get_str(mode);

    //TODO : handle 'x' mode (aka throw an error if the file already exist).
    bool read = false;
    bool write = false;
    bool binary = false;
    bool edit = false;
    bool append = false;
    for(int i = 0; c_mode[i] != 0; i++) {
        char c = c_mode[i];
        if(c == 'w') {
            write = true;
        }
        else if(c == 'r') {
            read = true;
        }
        else if(c == 'b') {
            binary = true;
        }
        else if(c == '+') {
            edit = true;
            read = true;
            write = true;
        }
        else if(c == 'a') {
            append = true;
            write = true;
        }
    }


    FILE* f = fopen(c_path, c_mode);
    if(f == NULL) {
        mp_raise_OSError(MP_ENOENT);    //no such file or directory
    }

    nds_obj_nitro_file* fileobj = m_new_obj(nds_obj_nitro_file);
    fileobj->base.type = &nds_type_nitrofile;

    fileobj->fileptr = f;
    fileobj->read = read;
    fileobj->write = write;
    fileobj->binary = binary;
    //TODO : edit and append mode
    (void)append;
    (void)edit;

    fseek(f, 0, SEEK_END);
    fileobj->charcount = ftell(f);
    fseek(f, 0, SEEK_SET);

    return MP_OBJ_FROM_PTR(fileobj);
}
MP_DEFINE_CONST_FUN_OBJ_2(nds_open_obj, py_nds_open);







///////////////////////////////////////////////////////////////////////////
//// Define the module itself


//List all the things in the nds module
STATIC const mp_rom_map_elem_t mp_module_nds_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__)         , MP_ROM_QSTR(MP_QSTR_nds) },               //module name "nds"

    { MP_ROM_QSTR(MP_QSTR_printstackusage)  , MP_ROM_PTR(&nds_printstackusage_obj) },   //function "printstackusage()"
    { MP_ROM_QSTR(MP_QSTR_nitrofsinit)      , MP_ROM_PTR(&nds_nitrofsinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_open)             , MP_ROM_PTR(&nds_open_obj) },

    { MP_ROM_QSTR(MP_QSTR_myconst)          , myconst },

    //{ MP_ROM_QSTR(MP_QSTR_NitroFile)        , MP_ROM_PTR(&nds_type_nitrofile) },        //class "NitroFile"
};
STATIC MP_DEFINE_CONST_DICT(mp_module_nds_globals, mp_module_nds_globals_table);


//Module object
const mp_obj_module_t mp_module_nds = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mp_module_nds_globals,
};

MP_REGISTER_MODULE(MP_QSTR_nds, mp_module_nds);
