#include <time.h>
#include <ewoksys/kernel_tic.h>

int clock_gettime(clockid_t clk_id, struct timespec *tp) {
    (void)clk_id;
    if(tp == NULL)
        return -1;
    
    uint64_t ms = kernel_tic_ms(0);
    tp->tv_sec = ms / 1000;
    tp->tv_nsec = (ms % 1000) * 1000000;
    return 0;
}
