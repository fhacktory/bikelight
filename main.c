/**
*
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "screen/screen.h"
#include "mpu/mpu.h"
#include "clock/clock.h"
//-- Private functions
void main_init(void);
void main_compute(void);
int main_escape(void);

/**
* 
*/
int main (int argc, char *argv[]){
	main_init();

	while(1)
	{
		main_compute();
		if(main_escape())
		{
			break;
		}
	}
	return 0;
};


void main_init()
{
	printf("App. Initialized...");
	mpu_init();
	screen_init();
	printf("OK\n");
}


void main_compute()
{
	clock_refreshTimer();
	mpu_compute();
	screen_compute();
}

/**
* Check if the programme need to be stop
* @return int 0 : Programme should continue running
			  1 : Programme need to be killed
*/
int main_escape()
{
	return 0;
}