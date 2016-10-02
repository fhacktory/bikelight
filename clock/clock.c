#include <time.h>
#include <stdint.h>
#include "clock.h"

struct timespec sActualClock, sPrevClock;
uint64_t ui64DeltaClock;


void clock_init()
{
	clock_gettime(CLOCK_MONOTONIC, &sActualClock);	/* mark start time */
	sPrevClock = sActualClock;
	ui64DeltaClock = 0;
}

void clock_refreshTimer()
{
	sPrevClock = sActualClock;
	clock_gettime(CLOCK_MONOTONIC, &sActualClock);	/* mark start time */
	if(sActualClock.tv_nsec < sPrevClock.tv_nsec)
	{
		ui64DeltaClock =   	0xFFFFFFFFFFFFFFFF - (sPrevClock.tv_nsec - sActualClock.tv_nsec);
	}
	else
	{
		ui64DeltaClock =  sActualClock.tv_nsec - sPrevClock.tv_nsec;
	}
}
