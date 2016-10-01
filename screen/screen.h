

#ifndef SCREEN_INCLUDE_H
#define SCREEN_INCLUDE_H

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

#define SIN 0
#define SCLK 4
#define XLAT 2
#define BLANK 3

#define DELAY 0

//-- Public functions

void screen_init(void);
void screen_compute(void);

#endif