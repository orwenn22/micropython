#include "py/builtin.h"

mp_uint_t mp_hal_ticks_ms(void) {
    // TODO return cycle count
    return 0;
}

// taken from some other port, idk
void mp_hal_delay_ms(mp_uint_t ms) {
    for (int i = ms; i > 0; i--) {
        for (volatile int j = 0; j < 5000; j++) {
        }
    }
}

void mp_hal_delay_us(mp_uint_t delay) {
    return; //TODO
}

mp_uint_t mp_hal_ticks_us(void) {
    return 42;  //TODO 
}

mp_uint_t mp_hal_ticks_cpu(void) {
    return 42;  //TODO
}