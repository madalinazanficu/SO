#include <unistd.h>
#include <internal/syscall.h>
#include <time.h>

int nanosleep(const struct timespec *req, struct timespec *rem) {
    return syscall(__NR_nanosleep, req, rem);
}