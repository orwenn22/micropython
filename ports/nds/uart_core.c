#include <unistd.h>
#include "py/mpconfig.h"

#include <nds.h>
#include <stdio.h>

/*
 * Core UART functions to implement for a port
 */


// Receive single character
int mp_hal_stdin_rx_chr(void) {
    scanKeys();     //We need to scanKey before getting the key from the keyboard
    int key = keyboardUpdate();
    if(key > 0) {
        if(key == '\n') {       //libnds' keyboard return '\n' when enter is pressed...
            return '\r';        //...but micropython's readline only accept '\r'
        }
        return key;
    }
    return 0;
}

// Send string of given length
void mp_hal_stdout_tx_strn(const char *str, mp_uint_t len) {
    //int r = write(STDOUT_FILENO, str, len);
    int r = fwrite(str, 1, len, stdout);
    (void)r;
}