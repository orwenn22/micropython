#include "py/builtin.h"
#include "py/runtime.h"
#include "py/stackctrl.h"
#include "py/mperrno.h"
#include "py/obj.h"
#include "py/objstr.h"

#include "nitrofile.h"

#include <dirent.h>
#include <stdio.h>
#include <unistd.h>


///////////////////////////
// NitroFile.close()
static mp_obj_t py_nds_nitrofile_close(mp_obj_t self_in) {
    nds_obj_nitro_file* self = MP_OBJ_TO_PTR(self_in);
    if(self->fileptr != NULL) {
        fclose(self->fileptr);
        self->fileptr = NULL;
    } else {
        printf("File ptr is null\n");
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(nds_nitrofile_close_obj, py_nds_nitrofile_close);


///////////////////////////
// NitroFile.ptr() -> Int
static mp_obj_t py_nds_nitrofile_ptr(mp_obj_t self_in) {
    nds_obj_nitro_file* self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int((unsigned long)self->fileptr);
}
MP_DEFINE_CONST_FUN_OBJ_1(nds_nitrofile_ptr_obj, py_nds_nitrofile_ptr);


///////////////////////////
// NitroFile.readable() -> Bool
static mp_obj_t py_nds_nitrofile_readable(mp_obj_t self_in) {
    nds_obj_nitro_file* self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_bool(self->read);
}
MP_DEFINE_CONST_FUN_OBJ_1(nds_nitrofile_readable_obj, py_nds_nitrofile_readable);


///////////////////////////
// NitroFile.write(data : str)
static mp_obj_t py_nds_nitrofile_write(mp_obj_t self_in, mp_obj_t str_data) {
    nds_obj_nitro_file* self = MP_OBJ_TO_PTR(self_in);

    if(self->write == false) {
        mp_raise_OSError(EIO);  //idk if it's an IO error in that case
    }

    GET_STR_DATA_LEN(str_data, c_str, lenght);
    fwrite(c_str, 1, lenght, self->fileptr);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(nds_nitrofile_write_obj, py_nds_nitrofile_write);


///////////////////////////
// NitroFile.read() -> str
static mp_obj_t py_nds_nitrofile_read(size_t n_args, const mp_obj_t *args) {
    nds_obj_nitro_file* self = MP_OBJ_TO_PTR(args[0]);

    size_t finallength = self->charcount - ftell(self->fileptr);
    size_t alloc_size = 0;

    if(n_args == 2) {
        int char_count = mp_obj_get_int(args[1]);
        if(finallength > char_count) {
            finallength = char_count;
        }
    }
    if(finallength == 0) {
        return mp_obj_new_str("", 0);
    }


    if(self->binary) {
        alloc_size = finallength;
    }
    else {
        /*short c = getc(self->fileptr);
        size_t i = 0;
        while(c != EOF) {
            if(i >= finallength) break;
            i++;
            alloc_size++;
            c = getc(self->fileptr);
            if((c&80) != 0) {     //utf8
                c = getc(self->fileptr);
                alloc_size++;
            }
        }
        fseek(self->fileptr, pos_in_file, SEEK_SET);*/
        size_t pos_in_file = ftell(self->fileptr);
        for(size_t i = 0; i < finallength; i++) {
            unsigned char c = getc(self->fileptr);
            alloc_size++;
            if((c&0x80) != 0) {
                c = getc(self->fileptr);
                alloc_size++;
            }
        }
        fseek(self->fileptr, pos_in_file, SEEK_SET);
    }

    char* c_rstr = m_malloc(alloc_size);   //NOTE: this is not terminated by a null character.
    fread(c_rstr, 1, alloc_size, self->fileptr);

    mp_obj_t py_obj;
    if(self->binary) {
        py_obj = mp_obj_new_bytearray(finallength, c_rstr);
    }
    else {
        py_obj = mp_obj_new_str(c_rstr, alloc_size);
    }
    m_free(c_rstr);
    return py_obj;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(nds_nitrofile_read_obj, 1, 2, py_nds_nitrofile_read);


///////////////////////////
// NitroFile.readline() -> str
static mp_obj_t py_nds_nitrofile_readline(mp_obj_t self_in) {
    nds_obj_nitro_file* self = MP_OBJ_TO_PTR(self_in);

    //Find the next end of line or end of file
    size_t currentseek = ftell(self->fileptr);
    short c = 0;
    do {
        c = getc(self->fileptr);
    } while(c != EOF && c != '\n');
    size_t newseek = ftell(self->fileptr);

    fseek(self->fileptr, currentseek, SEEK_SET);

    //Calculate the lenght requiered to store the string
    size_t lenght = newseek-currentseek;
    if(lenght <= 0) {
        return mp_obj_new_str("", 0);
    }

    //allocate the memory and fill the string
    char* c_str = m_malloc(lenght);
    for(size_t i = 0; i < lenght; i++) {
        c_str[i] = getc(self->fileptr);
    }

    //create the python string
    mp_obj_t py_obj;
    if(self->binary) {
        py_obj = mp_obj_new_bytearray(lenght, c_str);
    }
    else {
        py_obj = mp_obj_new_str(c_str, lenght);
    }
    m_free(c_str);
    return py_obj;
}
MP_DEFINE_CONST_FUN_OBJ_1(nds_nitrofile_readline_obj, py_nds_nitrofile_readline);


///////////////////////////
// NitroFile.readlines() -> List[str]
static mp_obj_t py_nds_nitrofile_readlines(mp_obj_t self_in) {
    //nds_obj_nitro_file* self = MP_OBJ_TO_PTR(self_in);

    mp_obj_t py_list = mp_obj_new_list(0, NULL);

    while(true) {
        mp_obj_t py_line = py_nds_nitrofile_readline(self_in);
        if(mp_obj_str_equal(py_line, mp_obj_new_str("", 0))) {  //check for empty string (end of file)
            break;
        }
        mp_obj_list_append(py_list, py_line);
    }
    return py_list;
}
MP_DEFINE_CONST_FUN_OBJ_1(nds_nitrofile_readlines_obj, py_nds_nitrofile_readlines);


///////////////////////////
// NitroFile.seek(offset [, whence])
static mp_obj_t py_nds_nitrofile_seek(size_t n_args, const mp_obj_t *args) {
    nds_obj_nitro_file* self = MP_OBJ_TO_PTR(args[0]);
    int offset = mp_obj_get_int(args[1]);
    int whence = 0;

    if(n_args == 3) {
        whence = mp_obj_get_int(args[2]);
    }

    //TODO : this is not how this is handled in CPython.
    return mp_obj_new_int(fseek(self->fileptr, offset, whence));
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(nds_nitrofile_seek_obj, 2, 3, py_nds_nitrofile_seek);


///////////////////////////
// NitroFile.tell() -> str
static mp_obj_t py_nds_nitrofile_tell(mp_obj_t self_in) {
    nds_obj_nitro_file* self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(ftell(self->fileptr));
}
MP_DEFINE_CONST_FUN_OBJ_1(nds_nitrofile_tell_obj, py_nds_nitrofile_tell);


///////////////////////////
// NitroFile.__enter__([...]) //TODO : not functional
static mp_obj_t py_nds_nitrofile___enter__(mp_obj_t self_in) {
    return self_in;
}
MP_DEFINE_CONST_FUN_OBJ_1(nds_nitrofile___enter___obj, py_nds_nitrofile___enter__);


///////////////////////////
// NitroFile.__exit__([...])
static mp_obj_t py_nds_nitrofile___exit__(size_t n_args, const mp_obj_t *args) {
    (void)n_args;
    return py_nds_nitrofile_close(args[0]);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_nds_nitrofile___exit___obj, 4, 4, py_nds_nitrofile___exit__);


/////////////////////////////////////////////////
//// Define NitroFile type for micropython

//Methods & stuff in NitroFile
static const mp_rom_map_elem_t nds_nitrofile_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_close)     , MP_ROM_PTR(&nds_nitrofile_close_obj) },
    { MP_ROM_QSTR(MP_QSTR_ptr)       , MP_ROM_PTR(&nds_nitrofile_ptr_obj) },
    { MP_ROM_QSTR(MP_QSTR_readable)  , MP_ROM_PTR(&nds_nitrofile_readable_obj) },
    { MP_ROM_QSTR(MP_QSTR_write)     , MP_ROM_PTR(&nds_nitrofile_write_obj) },
    { MP_ROM_QSTR(MP_QSTR_read)      , MP_ROM_PTR(&nds_nitrofile_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_readline)  , MP_ROM_PTR(&nds_nitrofile_readline_obj) },
    { MP_ROM_QSTR(MP_QSTR_readlines) , MP_ROM_PTR(&nds_nitrofile_readlines_obj) },
    { MP_ROM_QSTR(MP_QSTR_seek)      , MP_ROM_PTR(&nds_nitrofile_seek_obj) },
    { MP_ROM_QSTR(MP_QSTR_tell)      , MP_ROM_PTR(&nds_nitrofile_tell_obj) },
    { MP_ROM_QSTR(MP_QSTR___enter__) , MP_ROM_PTR(&nds_nitrofile___enter___obj) },
    { MP_ROM_QSTR(MP_QSTR___exit__)  , MP_ROM_PTR(&py_nds_nitrofile___exit___obj) },
};
static MP_DEFINE_CONST_DICT(nds_nitrofile_dict, nds_nitrofile_dict_table);  //Convert it to micropython object (dict)

//Object "NitroFile" definition
MP_DEFINE_CONST_OBJ_TYPE(
    nds_type_nitrofile,                 //type definition
    MP_QSTR_NitroFile,                  //name "NitroFile"
    MP_TYPE_FLAG_NONE,                  //idk
    locals_dict, &nds_nitrofile_dict    //link to all methods
);