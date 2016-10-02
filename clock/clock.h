#ifndef CLOCK_INCLUDE_D
#define CLOCK_INCLUDE_D

#include <time.h>

#define BILLION 1000000000L

extern uint64_t ui64DeltaClock;


void clock_init(void);
void clock_refreshTimer(void);


#endif //CLOCK_INCLUDE_D