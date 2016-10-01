#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wiringPi.h>
#include "../lib/tlc5947/tlc5947.h"
#include "screen.h"


/**
* Look at table of led setup in 8-shades of gray
*/
const int[] AUIN8LOOKATTABLELED = {0 , 3 , 11 , 35, 116, 380, 1248, 4095};


//-- Private prototype
uint16_t screen_getIntensity(ui8ShadeGray);
uint8_t screen_getWeightPixel(float fX, float fY);
void screen_getCoord(uint8_t ui8idLed, uint16_t ui16Angle, float *pfX, float *pfY)
//-- Private variables

uint16_t ui16Angle = 0;
s_BitMapScreen *psCurrentFrame;
uint16_t[SCREEN_RAIL_PIXEL] aui8Rail = [0, 0, 0, 0, 0, 0, 0, 0];
void screen_init()
{


	//Init Driver Gpio
	tlc5947init();
	tlc5947cleanup();
}

void screen_compute()
{
	screen_generateRailSequence();
	updateLeds();
} 




uint16_t screen_getIntensity(uint8_t ui8ShadeGray)
{
	return AUIN8LOOKATTABLELED[ui8ShadeGray];
}

/**
* Generate a new rail sequence
*/
void screen_generateRailSequence()
{
	uint8_t ui8IndexLed = 0;
	float fX, fY;
	for(ui8IndexLed = 0; ui8IndexLed < SCREEN_RAIL_PIXEL; ui8IndexLed++)
	{	
		screen_getCoord(ui8IndexLed, ui16Angle, &fX, &fY);
		tlcleds[ui8IndexLed] = screen_getIntensity(screen_getWeightPixel(fX, fY));
	}
}



void screen_getCoord(uint8_t ui8idLed, uint16_t ui16Angle, float *pfX, float *pfY)
{
	/*
	*    #4     |
	*           |
	*  Cadran   | #1
	*  270->360 | Cadran 0 -> 90
	*           |
	*           |
	*           |
	*  ====================
	*   #3      |  #2
	*    Cadran	| Cadran 90 -> 180
	*  180->270 |
	*           |
	*           |
	*           |
	*           |
	*/
	float fX = SCREEN_WIDTH, fY = SCREEN_HEIGHT;
	//Cadran 1
	if(SCREEN_IS_IN_QUARTER_NORTH_WEST(ui16Angle))
	{
		ui16Angle -= SCREEN_QUARTER_NORTH_WEST_ANGLE;
		fX -= cos(ui16Angle)*(ui8idLed + SCREEN_MIDDLE_OFF_PIXEL_RADIUS);
		fY -= sin(ui16Angle)*(ui8idLed + SCREEN_MIDDLE_OFF_PIXEL_RADIUS);	
	}
	else if(SCREEN_IS_IN_QUARTER_SOUTH_WEST(ui16Angle))
	{
		ui16Angle -= SCREEN_QUARTER_SOUTH_WEST_ANGLE;
		fX -= sin(ui16Angle)*(ui8idLed + SCREEN_MIDDLE_OFF_PIXEL_RADIUS);
		fY += cos(ui16Angle)*(ui8idLed + SCREEN_MIDDLE_OFF_PIXEL_RADIUS);	
	}
	else if(SCREEN_IS_IN_QUARTER_SOUTH_EAST(ui16Angle))
	{
		ui16Angle -= SCREEN_QUARTER_SOUTH_EAST_ANGLE;
		fX += cos(ui16Angle)*(ui8idLed + SCREEN_MIDDLE_OFF_PIXEL_RADIUS);
		fY += sin(ui16Angle)*(ui8idLed + SCREEN_MIDDLE_OFF_PIXEL_RADIUS);	
	}
	else
	{
		fX += sin(ui16Angle)*(ui8idLed + SCREEN_MIDDLE_OFF_PIXEL_RADIUS);
		fY -= cos(ui16Angle)*(ui8idLed + SCREEN_MIDDLE_OFF_PIXEL_RADIUS);	
	}

	pfX = fX;
	pfY = fY;
}


uint8_t screen_getWeightPixel(float fX, float fY)
{
	/*
		A___________B
		|			|
		| M      	|
		D___________C

	*/
	float fXfloor = floor(fX);
	float fYfloor = floor(fY);
	float fXceil = fXfloor + 1;
	float fYceil = fYfloor + 1;
	float dA = sqrtf((fX-fXfloor)*(fX-fXfloor) +  (fY-fYfloor)* (fY-fYfloor));
	float dB = sqrtf((fXceil - fX)*(fXceil - fX) +  (fY-fYfloor)* (fY-fYfloor));
	float dC = sqrtf((fXceil - fX)*(fXceil - fX) +  (fYceil - fY)* (fYceil - fY));
	float dD = sqrtf((fX-fXfloor)*(fX-fXfloor) +  (fYceil - fY)* (fYceil - fY));

	float dSum = dA + dB + dC + dD;
	uint8_t ui8XTopLeftCorner = (uint8_t) fXfloor;
	uint8_t ui8YTopLeftCorner = (uint8_t) fYfloor;
	return uint8_t (((float) dC*psCurrentFrame[ui8YTopLeftCorner][ui8XTopLeftCorner] + dD*psCurrentFrame[ui8YTopLeftCorner][ui8XTopLeftCorner + 1] + dA*psCurrentFrame[ui8YTopLeftCorner + 1][ui8XTopLeftCorner + 1] + dB*psCurrentFrame[ui8YTopLeftCorner + 1][ui8XTopLeftCorner])/dSum); 
}
