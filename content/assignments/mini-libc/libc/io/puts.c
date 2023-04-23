#include <unistd.h>
#include <internal/syscall.h>
#include <stdarg.h>
#include <errno.h>
#include <stdio.h>

int puts(const char *s) {

    while (*s != '\0') {
        if (write(1, s, 1) != 1) {
            return -1;
        }
        s++;
    }
    if (write(1, "\n", 1) != 1) {
        return -1;
    }

    return 1;
}