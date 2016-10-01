#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wiringPi.h>
#include "../lib/tlc5947/tlc5947.h"
#include "screen.h"



void screen_init()
{


	//Init Driver Gpio
	tlc5947init();
	tlc5947cleanup();

}

void screen_compute()
{
	/*Test Led All led*/
	setAllLeds(4095);
	updateLeds();

	delay(2500);

	tlc5947cleanup();
	delay(2500);

	printf("Test rail led off\n");

} 

