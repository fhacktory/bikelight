

#ifndef SCREEN_INCLUDE_H
#define SCREEN_INCLUDE_H

#include <stdint.h>

/*
 * Basic Use:
 * Call tlc5947init() before trying to control the chip.
 * Use setLed, setAllLeds, and clearLeds to change the outputs.
 * Once you're happy with the values of each output, call updateLeds() to send this information to the chip.
 * Once you're done, call tlc5947cleanup().
 *
 * To use more than one TLC5947, modify NUM_TLCS in the header file and recompile.
 *
 * Pin Mapping:
 * 
 * 0 -> SIN
 * 4 -> SCLK
 * 2 -> XLAT
 * 3 -> BLANK
 *
 * Note: The above follows WiringPi's numbering convention, just google it or something.
 */

//-- Define

/**
* Dimensionnal rules
*/
#define SCREEN_RAIL_PIXEL	16
#define SCREEN_MIDDLE_OFF_PIXEL 38
#define SCREEN_MIDDLE_OFF_PIXEL_RADIUS (SCREEN_MIDDLE_OFF_PIXEL / 2)
#define SCREEN_WIDTH (2*SCREEN_RAIL_PIXEL + SCREEN_MIDDLE_OFF_PIXEL)
#define SCREEN_HEIGHT (2*SCREEN_RAIL_PIXEL + SCREEN_MIDDLE_OFF_PIXEL)

#define SCREEN_QUARTER_NORTH_EAST_ANGLE	0
#define SCREEN_QUARTER_SOUTH_EAST_ANGLE	90
#define SCREEN_QUARTER_SOUTH_WEST_ANGLE	180
#define SCREEN_QUARTER_NORTH_WEST_ANGLE	270

#define SCREEN_IS_IN_QUARTER_NORTH_EAST(angle) (angle >= SCREEN_QUARTER_NORTH_EAST_ANGLE)
#define SCREEN_IS_IN_QUARTER_SOUTH_EAST(angle) (angle >= SCREEN_QUARTER_SOUTH_EAST_ANGLE)
#define SCREEN_IS_IN_QUARTER_SOUTH_WEST(angle) (angle >= SCREEN_QUARTER_SOUTH_WEST_ANGLE)
#define SCREEN_IS_IN_QUARTER_NORTH_WEST(angle) (angle >= SCREEN_QUARTER_NORTH_WEST_ANGLE)
/**
* Pinout
*/
#define SIN 0
#define SCLK 4
#define XLAT 2
#define BLANK 3

#define DELAY 0






//-- Type

 typedef struct{
 	uint8_t aui8Bitmap[SCREEN_HEIGHT][SCREEN_WIDTH];
}s_BitMapScreen;

//-- Public functions

void screen_init(void);
void screen_compute(void);

#endif