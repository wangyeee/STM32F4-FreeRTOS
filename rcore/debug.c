/* debug.c
 * Debugging facilities, including panic()
 * RebbleOS
 *
 * Author: Joshua Wise <joshua@joshuawise.com>
 */

#include <stdio.h>
#include "debug.h"

void panic(const char *s) {
    puts("*** PANIC ***");
    puts(s);
    while (1)
        ;
    /* XXX: do something smarter here, like turn IRQs off and stop poking the watchdog */
}
