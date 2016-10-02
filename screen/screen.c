#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wiringPi.h>
#include <stdbool.h>
#include <math.h>
#include "../lib/tlc5947/tlc5947.h"
#include "../clock/clock.h"
#include "screen.h"
#include "frame.h"

/**
* Look at table of led setup in 8-shades of gray
*/
const int AUIN8LOOKATTABLELED[] = {0 , 3 , 11 , 35, 116, 380, 1248, 4095};


//-- Private prototype
void screen_initImage(void);
uint16_t screen_getIntensity(uint8_t ui8ShadeGray);
uint8_t screen_getWeightPixel(double dX, double dY);
void screen_getCoord(uint8_t ui8idLed, double dAngle, double *pdX, double *pdY);
void screen_generateRailSequence(void);
bool screen_checkFPS(void);
void screen_getNextImage(void);

//-- Private variables

double dAngle = 0.0;
s_BitMapScreen *psCurrentFrame;
s_AnimatedBitMapScreen *psAnimatedBitMapScreen;
uint16_t ui16CurrentFrame = 0;
uint16_t aui8Rail[] = {0, 0, 0, 0, 0, 0, 0, 0};
bool bFireStop = false;
uint64_t ui64ListTimeFrameRefresh = 0;



void screen_init()
{


	//Init Driver Gpio
	tlc5947init();
	tlc5947cleanup();
	screen_initImage();
}

void screen_compute()
{
	if(screen_checkFPS())
	{
		screen_getNextImage();
	}
	if(bFireStop)
	{
		return;
	}
	screen_generateRailSequence();
	updateLeds();
} 

void screen_initImage()
{
	psAnimatedBitMapScreen = &sAnimation;
	psCurrentFrame =  &psAnimatedBitMapScreen->apsBitMapScreen[0];
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
	double dX, dY;
	for(ui8IndexLed = 0; ui8IndexLed < SCREEN_RAIL_PIXEL; ui8IndexLed++)
	{	
		screen_getCoord(ui8IndexLed, dAngle, &dX, &dY);
		//printf("%2.1f=>%2.1f ",dX, dY);
		tlcleds[ui8IndexLed] = screen_getIntensity(screen_getWeightPixel(dX, dY));
		//printf("[%2.1f,%2.1f] \t%i=>%i\t ", dX, dY, ui8IndexLed, tlcleds[ui8IndexLed]);
		//printf("%i=>%i ",ui8IndexLed, tlcleds[ui8IndexLed]);
	}
	//printf("\n");
}



void screen_getCoord(uint8_t ui8idLed, double dAngle, double *pdX, double *pdY)
{
	/*
	*    #4     0
	*           |
	*  Cadran   | #1
	*  270->360 | Cadran 0 -> 90
	*           |
	*           |
	*           |
	* 270================== 90
	*   #3      |  #2
	*    Cadran	| Cadran 90 -> 180
	*  180->270 |
	*           |
	*           |
	*           |
	*           |
			   180
	*/		
	double dX = SCREEN_WIDTH / 2, dY = SCREEN_HEIGHT / 2;
	//Cadran 1
	if(SCREEN_IS_IN_QUARTER_NORTH_WEST(dAngle))
	{
		dAngle -= SCREEN_QUARTER_NORTH_WEST_ANGLE;
		dX -= cos(dAngle)*(ui8idLed + SCREEN_MIDDLE_OFF_PIXEL_RADIUS);
		dY -= sin(dAngle)*(ui8idLed + SCREEN_MIDDLE_OFF_PIXEL_RADIUS);	
	}
	else if(SCREEN_IS_IN_QUARTER_SOUTH_WEST(dAngle))
	{
		dAngle -= SCREEN_QUARTER_SOUTH_WEST_ANGLE;
		dX -= sin(dAngle)*(ui8idLed + SCREEN_MIDDLE_OFF_PIXEL_RADIUS);
		dY += cos(dAngle)*(ui8idLed + SCREEN_MIDDLE_OFF_PIXEL_RADIUS);	
	}
	else if(SCREEN_IS_IN_QUARTER_SOUTH_EAST(dAngle))
	{
		dAngle -= SCREEN_QUARTER_SOUTH_EAST_ANGLE;
		dX += cos(dAngle)*(ui8idLed + SCREEN_MIDDLE_OFF_PIXEL_RADIUS);
		dY += sin(dAngle)*(ui8idLed + SCREEN_MIDDLE_OFF_PIXEL_RADIUS);	
	}
	else
	{
		dX += sin(dAngle)*(ui8idLed + SCREEN_MIDDLE_OFF_PIXEL_RADIUS);
		dY -= cos(dAngle)*(ui8idLed + SCREEN_MIDDLE_OFF_PIXEL_RADIUS);	
	}

	*pdX = dX;
	*pdY = dY;
}


uint8_t screen_getWeightPixel(double dX, double dY)
{
	/*
		A___________B
		|			|
		| M      	|
		D___________C

	*/
	double dXfloor = floor(dX);
	double dYfloor = floor(dY);
	double dXceil = dXfloor + 1;
	double dYceil = dYfloor + 1;
	double dA = sqrt((dX - dXfloor)*(dX - dXfloor) +  (dY-dYfloor)* (dY-dYfloor));
	double dB = sqrt((dX - dXceil)*(dX - dXceil) +  (dY-dYfloor)* (dY-dYfloor));
	double dC = sqrt((dX - dXceil)*(dX - dXceil) +  (dY-dYceil)* (dY-dYceil));
	double dD = sqrt((dX - dXfloor)*(dX - dXfloor) +  (dY-dYceil)* (dY-dYceil));

	double dSum = dA + dB + dC + dD;
	uint8_t ui8XTopLeftCorner = (uint8_t) dXfloor;
	uint8_t ui8YTopLeftCorner = (uint8_t) dYfloor;
	uint8_t ui8Weight = 0;
	if(dX <= dXfloor + 0.5)
	{
		if(dY <= (dYfloor + 0.5))
		{
			ui8Weight = psCurrentFrame->aui8Bitmap[ui8YTopLeftCorner][ui8XTopLeftCorner];
		}
		else
		{
			ui8Weight = psCurrentFrame->aui8Bitmap[ui8YTopLeftCorner + 1][ui8XTopLeftCorner];
		}
	}
	else
	{
		if(dY <= (dYfloor + 0.5))
		{
			ui8Weight = psCurrentFrame->aui8Bitmap[ui8YTopLeftCorner][ui8XTopLeftCorner + 1];
		}
		else
		{
			ui8Weight = psCurrentFrame->aui8Bitmap[ui8YTopLeftCorner + 1][ui8XTopLeftCorner + 1];
		}
	}
	return ui8Weight;

	//return (uint8_t) (((double) dC*(psCurrentFrame->aui8Bitmap[ui8YTopLeftCorner][ui8XTopLeftCorner]) + dD*(psCurrentFrame->aui8Bitmap[ui8YTopLeftCorner][ui8XTopLeftCorner + 1]) + dA*(psCurrentFrame->aui8Bitmap[ui8YTopLeftCorner + 1][ui8XTopLeftCorner + 1]) + dB*(psCurrentFrame->aui8Bitmap[ui8YTopLeftCorner + 1][ui8XTopLeftCorner]))/dSum); 
}

bool screen_checkFPS()
{
	ui64ListTimeFrameRefresh += ui64DeltaClock;
	if(ui64ListTimeFrameRefresh >= psAnimatedBitMapScreen->ui64NanosecondsPerFrame)
	{
		ui64ListTimeFrameRefresh = 0;
		return true;
	}
	return false;
}

void screen_getNextImage()
{
	printf("Next Frame required [%i=>%i]\n",ui16CurrentFrame,ui16CurrentFrame+1);
	if(++ui16CurrentFrame >= psAnimatedBitMapScreen->ui16FrameAmount)
	{
		if(psAnimatedBitMapScreen->bIsLooping == true)
		{
			ui16CurrentFrame = 0;
		}
		else
		{
			bFireStop = true;
			return ;
		}
	}
	psCurrentFrame = (psAnimatedBitMapScreen->apsBitMapScreen[ui16CurrentFrame]);
	printf("%lld : %i=>%i%i%i%i%i%i\n",psCurrentFrame, ui16CurrentFrame, psCurrentFrame->aui8Bitmap[0][0],psCurrentFrame->aui8Bitmap[1][0],psCurrentFrame->aui8Bitmap[2][0],psCurrentFrame->aui8Bitmap[3][0],psCurrentFrame->aui8Bitmap[4][0],psCurrentFrame->aui8Bitmap[5][0]);
}

