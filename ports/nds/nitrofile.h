#ifndef NITROFILE_H
#define NITROFILE_H

#include "py/obj.h"
#include "py/runtime.h"

#include <stdio.h>  //for FILE*


// NitroFile object
typedef struct _nds_obj_nitro_file {
    mp_obj_base_t base;
    FILE* fileptr;
    bool read;          //true if the file is opened in read mode  ("r")
    bool binary;        //true if the file is opened in binary mode ("b")
    //TODO : edit and append mode
    size_t charcount;
    //size_t seekpos;
} nds_obj_nitro_file;

extern const mp_obj_type_t nds_type_nitrofile; //forward declaration , defined in nitrofile.c

#endif  //NITROFILE_H