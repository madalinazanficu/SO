#include <unistd.h>
#include <internal/syscall.h>
#include <internal/types.h>


unsigned int sleep(unsigned int seconds) {
    // int seconds_left = syscall(__NR_sleep, seconds);
    // return seconds_left;
}