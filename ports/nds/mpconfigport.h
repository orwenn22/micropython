#include <stdint.h>
#include <unistd.h>

// options to control how MicroPython is built

// Use the minimal starting configuration (disables all optional features).
#define MICROPY_CONFIG_ROM_LEVEL (MICROPY_CONFIG_ROM_LEVEL_EVERYTHING)

// You can disable the built-in MicroPython compiler by setting the following
// config option to 0.  If you do this then you won't get a REPL prompt, but you
// will still be able to execute pre-compiled scripts, compiled with mpy-cross.
#define MICROPY_ENABLE_COMPILER     (1)

#define MICROPY_QSTR_EXTRA_POOL           mp_qstr_frozen_const_pool
#define MICROPY_ENABLE_GC                 (1)
#define MICROPY_HELPER_REPL               (1)
#define MICROPY_MODULE_FROZEN_MPY         (1)
#define MICROPY_ENABLE_EXTERNAL_IMPORT    (1)

#define MICROPY_USE_INTERNAL_PRINTF (0)     //Pour ne pas utiliser le printf de shared/libc
#define MICROPY_PY_SYS_STDIO_BUFFER (0)

#define MICROPY_ALLOC_PATH_MAX            (256)
#define MICROPY_ALLOC_PARSE_CHUNK_INIT    (16)

#define MICROPY_VFS                         (0)     //tkt
#define MICROPY_ENABLE_FINALISER (1)

#define MICROPY_PY_SYS_STDFILES     (0)

#define MICROPY_PY_OS (0)

//possiblement pas nécessaire
//#ifndef MICROPY_STACKLESS
//#define MICROPY_STACKLESS           (0)
//#define MICROPY_STACKLESS_STRICT    (0)
//#endif

//lib math
#define MICROPY_FLOAT_IMPL (1)
#define MICROPY_PY_MATH (1)

//lib random
#define MICROPY_PY_URANDOM              (1)
#define MICROPY_PY_URANDOM_EXTRA_FUNCS  (1)

//lib time
#define MICROPY_PY_UTIME (1)        //TODO : apparemment il faut impémenter son propre truc time, prendre exemple du fichier modutime des autres ports


//FIXME : ça marche pas vraiment lol
#define MICROPY_PY_BUILTINS_HELP (1)

// type definitions for the specific machine

typedef intptr_t mp_int_t; // must be pointer size
typedef uintptr_t mp_uint_t; // must be pointer size
typedef long mp_off_t;

// We need to provide a declaration/definition of alloca()
#include <alloca.h>

#define MICROPY_HW_BOARD_NAME "Nintendo DS"
#define MICROPY_HW_MCU_NAME "arm946e-s"

/*
#ifdef __linux__
#define MICROPY_MIN_USE_STDOUT (1)
#define MICROPY_HEAP_SIZE      (25600) // heap size 25 kilobytes
#endif

#ifdef __thumb__
#define MICROPY_MIN_USE_CORTEX_CPU (1)
#define MICROPY_MIN_USE_STM32_MCU (1)
#define MICROPY_HEAP_SIZE      (2048) // heap size 2 kilobytes
#endif
*/


#define MICROPY_HEAP_SIZE      (1024*1024) // heap size 1 megabytes

#define MICROPY_PY_SYS_PLATFORM "nds"

//taken from the unix port
#ifndef MICROPY_EVENT_POLL_HOOK
#define MICROPY_EVENT_POLL_HOOK \
    do { \
        extern void mp_handle_pending(bool); \
        mp_handle_pending(true); \
        usleep(500); /* equivalent to mp_hal_delay_us(500) */ \
    } while (0);
#endif

#define MP_STATE_PORT MP_STATE_VM
