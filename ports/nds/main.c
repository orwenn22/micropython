#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "py/builtin.h"
#include "py/compile.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/repl.h"
#include "py/stackctrl.h"
#include "py/gc.h"
#include "shared/runtime/pyexec.h"

#include <dirent.h>
#include <nds.h>
#include <fatfs.h>
#include <unistd.h>
#include <sys/stat.h>
#include <nds/arm9/dldi.h>


#if MICROPY_ENABLE_COMPILER
void do_str(const char *src, mp_parse_input_kind_t input_kind) {
    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        mp_lexer_t *lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, src, strlen(src), 0);
        qstr source_name = lex->source_name;
        mp_parse_tree_t parse_tree = mp_parse(lex, input_kind);
        mp_obj_t module_fun = mp_compile(&parse_tree, source_name, true);
        mp_call_function_0(module_fun);
        nlr_pop();
    } else {
        // uncaught exception
        mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);
    }
}
#endif


static char *stack_top;
#if MICROPY_ENABLE_GC
static char heap[MICROPY_HEAP_SIZE];
#endif

bool g_nitroenabled;

int main(int argc, char **argv) {
    lcdMainOnBottom();      //mettre main en bas et sub en haut
    consoleDemoInit();      //la console est initialisé sur le sub (en haut)
    printf("DLDI : %s\n", io_dldi_data->friendlyName);

    g_nitroenabled = nitroFSInit(NULL);
    if(g_nitroenabled) { chdir("nitro:/root"); }



    videoSetMode(MODE_0_2D);
    keyboardInit(NULL, 3, BgType_Text4bpp, BgSize_T_256x512, 20, 0, true, true);    //initialiser le clavier en bas
    keyboardShow();

    mp_stack_ctrl_init();
    mp_stack_set_limit(48 * 1024);  //48 ko

    printf("NitroFS      : %u\n", g_nitroenabled);
    printf("Stack size   : %u\n", 48 * 1024);
    printf("Heap size    : %u\n", MICROPY_HEAP_SIZE);
    printf("Heap address : %p\n", heap);


    #if MICROPY_ENABLE_GC
    gc_init(heap, heap + sizeof(heap));
    #endif
    mp_init();
    #if MICROPY_ENABLE_COMPILER

    //iprintf("Hello from C\n");
    //do_str("print('hello DS')", MP_PARSE_SINGLE_INPUT);
    //iprintf("Hello from C\n");
    
    //do_str("print('hello world!', list(x+1 for x in range(10)), end='eol\\n')", MP_PARSE_SINGLE_INPUT);
    //do_str("for i in range(10):\r\n  print(i)", MP_PARSE_FILE_INPUT);

    #if MICROPY_REPL_EVENT_DRIVEN
    pyexec_event_repl_init();
    for (;;) {
        int c = mp_hal_stdin_rx_chr();
        if (pyexec_event_repl_process_char(c)) {
            break;
        }
    }
    #else
    pyexec_friendly_repl();
    #endif
    // do_str("print('hello world!', list(x+1 for x in range(10)), end='eol\\n')", MP_PARSE_SINGLE_INPUT);
    // do_str("for i in range(10):\r\n  print(i)", MP_PARSE_FILE_INPUT);
    #else
    pyexec_frozen_module("frozentest.py");
    #endif
    mp_deinit();
    return 0;
}

#if MICROPY_ENABLE_GC
void gc_collect(void) {
    // WARNING: This gc_collect implementation doesn't try to get root
    // pointers from CPU registers, and thus may function incorrectly.
    void *dummy;
    gc_collect_start();
    gc_collect_root(&dummy, ((mp_uint_t)stack_top - (mp_uint_t)&dummy) / sizeof(mp_uint_t));
    gc_collect_end();
    gc_dump_info(&mp_plat_print);
}
#endif

//Ouvrir le fichier filename et en faire un lexer pour l'exécuter.
//Notamment utiliser par import.
mp_lexer_t *mp_lexer_new_from_file(qstr filename) {
    //mp_raise_OSError(MP_ENOENT);

    //Supposer que le fichier existe
    FILE* f = fopen(qstr_str(filename), "r");

    fseek(f, 0, SEEK_END);
    size_t lenght = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* data = m_malloc(lenght);      //NOTE : can't be deallocated with m_free()
    for(size_t i = 0; i < lenght; i++) {
        data[i] = getc(f);
    }
    fclose(f);

    return mp_lexer_new_from_str_len(filename, (const char *)data, (mp_uint_t)lenght, 0);
}

// Vérifier si le fichier d'importation existe ? Et savoir si c'est un dossier ou un fichier.
mp_import_stat_t mp_import_stat(const char *path) {
    //return MP_IMPORT_STAT_NO_EXIST;

    //printf("PATH : %s\n", path);

    struct stat filestat;
    stat(path, &filestat);
    
    if(S_ISREG(filestat.st_mode)) { //check if it's a file
        //printf("FILE\n");
        return MP_IMPORT_STAT_FILE;
    }


    DIR* d = opendir(path);
    if(d != NULL) {     //check if it's a directory
        closedir(d);
        //printf("DIRECTORY\n");
        return MP_IMPORT_STAT_DIR;
    }
    closedir(d);

    //printf("NO EXIST\n");
    return MP_IMPORT_STAT_NO_EXIST;
}


mp_obj_t py_nds_open(mp_obj_t path, mp_obj_t mode);     //forward declaration (TODO : move to header ?)
mp_obj_t mp_builtin_open(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
    //printf("Open called with %u arguments\n", n_args);
    mp_obj_t mode;
    bool ismodedefined = false;
    if(n_args == 2) {
        mode = args[1];
        ismodedefined = true;
    } 
    else {
        mp_map_elem_t* modeelem = mp_map_lookup(kwargs, mp_obj_new_str("mode", 4), MP_MAP_LOOKUP_ADD_IF_NOT_FOUND);
        //printf("mode : %s\n", mp_obj_str_get_str(modeelem->value));
        if(mp_obj_is_str(modeelem->value)) {
            mode = modeelem->value;
            ismodedefined = true;
        }
        else {
            mode = mp_obj_new_str("r", 1);
            ismodedefined = true;
        }
    }

    if(ismodedefined == true)
    return py_nds_open(args[0], mode);
    else return mp_obj_new_int(-1);
}
MP_DEFINE_CONST_FUN_OBJ_KW(mp_builtin_open_obj, 1, mp_builtin_open);



void nlr_jump_fail(void *val) {
    printf("Jump fail\n");
    printf("Stack usage : %u\n", mp_stack_usage());
    printf("Value : %p\n", val);
    while (1) {
        ;
    }
}

void NORETURN __fatal_error(const char *msg) {
    printf("FATAL ERROR\n Message : %s\n", msg);
    while (1) {
        ;
    }
}

#ifndef NDEBUG
void MP_WEAK __assert_func(const char *file, int line, const char *func, const char *expr) {
    printf("Assertion '%s' failed, at file %s:%d\n", expr, file, line);
    __fatal_error("Assertion failed");
}
#endif

