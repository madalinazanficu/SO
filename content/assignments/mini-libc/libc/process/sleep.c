#include <unistd.h>
#include <internal/syscall.h>
#include <time.h>

unsigned int sleep(unsigned int seconds) {

    struct timespec time_req;
    time_req.tv_sec = seconds;
    time_req.tv_nsec = 0;

    struct timespec time_rem;
    time_rem.tv_sec = 0;
    time_rem.tv_nsec = 0;

    return nanosleep(&time_req, &time_rem);
}