#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "stdint.h"
#include "math.h"
#include <time.h>

#define BILLION 1000000000L
uint64_t ui64DeltaClock;
void clock_init(void);
void clock_refreshTimer(void);

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

int adresse=0x68;//adresse du port I2C. Utiliser i2cdetect pour trouver cette adresse
long int timePrev, temps, timeStep;
//double arx, ary, arz, gsx, gsy, gsz, gry, grz, grx, rx, ry, rz;
float arx, ary, arz, gsx, gsy, gsz, gry, grz, grx, rx, ry, rz;
int fd;
int i=1;
//static const double gyroScale = 131; //131 LSB/deg/s
//static const double Rad2Degr=57.295779506;//PI=180
static const float gyroScale = 131; //131 LSB/deg/s
static const float Rad2Degr=57.295779506;//PI=180
int angleX, angleY, angleZ;
float angleX0, angleY0, angleZ0;

int conversionSigne(float valeur)
{
//le registre contient des donn  es sign  es (le LSB donne le signe)=>convertion sign  e
if (valeur>=0x8000)
return -((65535-valeur)+1);
else
return valeur;
}

float getAccX()
{
float temp;
//lecture des deux octets acc  l  ration sur le bus I2C
temp=wiringPiI2CReadReg8(fd,0x3B)<<8|wiringPiI2CReadReg8(fd,0x3C);
return conversionSigne(temp);
}

float getAccY()
{
float temp;
temp=wiringPiI2CReadReg8(fd,0x3D)<<8|wiringPiI2CReadReg8(fd,0x3E);
return conversionSigne(temp);
}

float getAccZ()
{
float temp;
temp=wiringPiI2CReadReg8(fd,0x3F)<<8|wiringPiI2CReadReg8(fd,0x40);
return conversionSigne(temp);
}

float getGyroX()
{
return wiringPiI2CReadReg8(fd,0x43)<<8|wiringPiI2CReadReg8(fd,0x44);
}

float getGyroY()
{
return wiringPiI2CReadReg8(fd,0x45)<<8|wiringPiI2CReadReg8(fd,0x46);
}

float getGyroZ()
{
return wiringPiI2CReadReg8(fd,0x47)<<8|wiringPiI2CReadReg8(fd,0x48);
}

void lireTous()
{
//lecture de tous les registres du capteur MPU6050
short int temp;
int i=0;
for (i=0x0D;i<0x76;i++)
{
temp=wiringPiI2CReadReg8(fd,i);
//qDebug()<<temp<<endl;
printf("%i\n",temp);
}
}

float getAngleX()
{
double temp;
double aX, aY, aZ;
aX=getAccX()/16384;//16.384 LSB pour 1g d'après la doc
aY=getAccY()/16384;
aZ=getAccZ()/16384;
temp=sqrt( pow(aY,2)
                 + pow(aZ,2));
return Rad2Degr * atan(aX/temp);
}

float getAngleY()
{
float temp;
float aX, aY, aZ;
aX=getAccX()/16384;//16.384 LSB pour 1g d'apr  s la doc
aY=getAccY()/16384;
aZ=getAccZ()/16384;
temp=sqrt( pow(aX,2)
               + pow(aZ,2));
return Rad2Degr * atan(aY/temp);
}

float getAngleZ()
{ //cette fonction n'a pas vraiment d'int  r  t si le capteur est pos      plat
//car il est impossible de trouver l'angle de rotation Z
//  tant donn   que l'acc  l  ration se d  termine    partir du vecteur gravitationnel
float temp;
float aX, aY, aZ;
aX=getAccX()/16384;//16.384 LSB pour 1g d'apr  s la doc
aY=getAccY()/16384;
aZ=getAccZ()/16384;
temp=sqrt( pow(aX,2)
              + pow(aY,2));
//return Rad2Degr*atan(temp/aZ);
return Rad2Degr*atan(aZ/temp);
}

void getAngles()
{ //cette fonction utilise le calcul d'angles par l'acc  l  ration
// puis int  gre les donn  es gyroscopiques par filtrage (filtre de Kalman) pour   viter les d  rives
// li  es    des chocs ou vibrations occasionnant des acc  l  rations
timePrev = temps;//on m  morise le temps initial pour Dt
temps = millis();
timeStep = (temps - timePrev) / 1000; // time-step in s
//r  cup des valeurs
arx=getAngleX();
ary=getAngleY();
arz=getAngleZ();
gsx=getGyroX()/gyroScale;
gsy=getGyroY()/gyroScale;
gsz=getGyroZ()/gyroScale;
if (i==1)
{ //initialement les valeurs gyro=valeurs accel
grx=arx;
gry=ary;
grz=arz;
}
else
{
grx=grx+(timeStep*gsx);
gry=gry+(timeStep*gsy);
grz=grz+(timeStep*gsz);
}
//delay(50);
delay(10);
//on filtre
angleX=(0.04*arx) +(0.96*grx);
angleY=(0.04*ary) +(0.96*gry);
angleZ=(0.04*arz) +(0.96*grz);
}

void initAngles()
{//d  termine les valeurs lorsque le capteur est au repos
//qDebug("Ne pas bouger et laisser le capteur en position d'  quilibre");
printf("Ne pas bouger et laisser le capteur en position d'equilibre\n");
float tx=0;
float ty=0;
float tz=0;
int i=0;
for (i=0;i<100;i++)
{
getAngles();
tx +=angleX;
ty +=angleY;
tz +=angleZ;
}
angleX0=tx/100;
angleY0=ty/100;
angleZ0=tz/100;
}

//-----AJOUT CODE-----//
#define DEGS_TO_ANGLE(x) (uint16_t) (x / 180.0f * (1uLL << 15))
#define DEG_PER_S_TO_RATE(x) (int16_t) (x * 16.4f)

static int32_t gyro_offset[3];
#define CALIB_SHIFT 7
static void zero_gyro(void) {
	int i;
	int reading[6];

	gyro_offset[0] = 0;
	gyro_offset[1] = 0;
	gyro_offset[2] = 0;

for (i = 0; i < (1 << CALIB_SHIFT); i ++) {
		reading[0]=getAccX();
		reading[1]=getAccY();
		reading[2]=getAccZ();
		reading[3]=getGyroX();
		reading[4]=getGyroY();
		reading[5]=getGyroZ();
		gyro_offset[0] += reading[3];
		gyro_offset[1] += reading[4];
		gyro_offset[2] += reading[5];
		delay(1);
	}
}

/* Update gyro values and timestamps, used for gyro integration */
static int16_t gyro_reading;
static int32_t gyro_reading_mult;
static uint32_t now, timediff;
#define MULT_BITS 4
//static void gyro_update(void) opts;
static void gyro_update(void) {
	gyro_reading_mult= (int32_t) getGyroZ()<<MULT_BITS;
	gyro_reading_mult -=
		(gyro_offset[2] +
		 (1 << (CALIB_SHIFT - MULT_BITS - 1))) >>
		(CALIB_SHIFT - MULT_BITS);
#ifndef REVERSE
	gyro_reading_mult = -gyro_reading_mult;
#endif
	gyro_reading = (gyro_reading_mult + (1 << (MULT_BITS - 1))) >>
		MULT_BITS;

	static uint32_t prev = 0;
	now = ui64DeltaClock/1000;  //Get systick
	timediff = now - prev;
	prev = now;
}

static struct {
	uint8_t prog;
	uint16_t gyro_mult;
	float cf_acc[2];
	uint8_t cf_samples;
}config;

static int16_t acc_reading[3];
int16_t acc[2];
static int16_t cf_acc_int[2];
//static void acc_update(void) opts;
static void acc_update(void) {
	acc_reading[0]=getAccX();
	acc_reading[1]=getAccY();
	acc_reading[2]=getAccZ();
	acc[0] = acc_reading[0];
	acc[1] = acc_reading[1];
	/*
	 * If cf calibration is based on reasonably many samples, use it for
	 * acc reading correction in gyro drift correction.
	 */
	if (config.cf_samples > 128) {
		uint16_t factor =
			((uint32_t) gyro_reading * gyro_reading) >> 20;
		acc[0] -= cf_acc_int[0] * factor;
		acc[1] -= cf_acc_int[1] * factor;
	}
}

uint16_t angle;
static uint16_t angle_update(void) {
int16_t step = ((int32_t) gyro_reading_mult * (int16_t) timediff) /
		(int32_t) (360.0f * 16.4f * 1000000.0f / 65536.0f + 0.499f);
	/*	            degs   lsb/deg    us/sec    lsb/360deg rounding */

	/* TODO: use optimised avr mult */
	step = ((int32_t) step * config.gyro_mult + (1 << (13 + MULT_BITS))) >>
		(14 + MULT_BITS);

	angle += step;

/*
	 * Centrifugal force estimation.  We sum (acceleration vector /
	 * rotation rate ^ 2) over time -- at least a few full turns at a
	 * high enough rotation rate, to find the direction of the force
	 * and the relation to rotation rate.  This way, knowing the rotation
	 * rate at any later point, we can calculate the centrifugal force
	 * vector and subtract it from accelerometer readings in order to
	 * better approximate the gravity vector.
	 *
	 * The MPU6050 gyro has a max range of 2000deg/s and this is a slightly
	 * faster than we expect a bicycle wheel to be able to turn, so we
	 * assume a practical range of our centrifugal force calculations to be
	 * 90deg/s - 2000deg/s.  We set a lower limit too because the force is
	 * proportional to the square of rotation rate and below a given value
	 * the force should be negligible.
	 */
	static uint16_t angle_accum;
	static uint8_t iter_accum;
	static uint32_t time_prev;

	angle_accum += abs(step);
	iter_accum += 1;
	if (iter_accum > 100 || now - time_prev > 150000) {
		acc_update();

		uint32_t len = ((int32_t) acc[0] * acc[0]) +
			((int32_t) acc[1] * acc[1]);
		uint8_t correct = len > 0x1000000 / 2 && len < 0x1000000 * 2;

		uint16_t acc_angle = atan2(acc[0], acc[1]) *
			(-32768.0f / M_PI);
		int16_t err_angle = acc_angle - angle;

		/* Correct the current angle */
		if (correct)
			angle += (err_angle + 4) >> 3;

		/* Correct the gyro zero offset (angle integral) */
#ifdef REVERSE
		err_angle = -err_angle;
#endif
		if (correct)
			gyro_offset[2] +=
				((int32_t) err_angle << 2) / iter_accum;

#if 0
		uint16_t tdiff = (now - time_prev) / 1000;
		//Serial.print((uint32_t) 1000 * iter_accum / tdiff);
		//printf("fps\r\n");
#endif
		time_prev = now;
		iter_accum = 0;

if (angle_accum > DEGS_TO_ANGLE(30.0f) &&
				abs(gyro_reading) > DEG_PER_S_TO_RATE(70.0f)) {
			/*
			 * Quite literally what is described in comment above.
			 * Use floats so we don't have to worry about ranges.
			 * Since this is only done every now and then, the
			 * overhead should be fine.
			 * TODO: increase the sample weight with rotation rate?
			 */
			config.cf_acc[0] += (acc_reading[0] /
					((float) gyro_reading * gyro_reading) -
					config.cf_acc[0]) * 0.01f;
			config.cf_acc[1] += (acc_reading[1] /
					((float) gyro_reading * gyro_reading) -
					config.cf_acc[1]) * 0.01f;
			if (config.cf_samples < 255)
				config.cf_samples += 1;

			cf_acc_int[0] = config.cf_acc[0] * (65536.0f * 16.0f);
			cf_acc_int[1] = config.cf_acc[1] * (65536.0f * 16.0f);

/*
			 * TODO: add phase shift between the cf calibration
			 * and acc-based gyro drift correction.
			 */

			/*
			 * Update gyro rate multiplier, use 1/32 weight (the
			 * 14-bit shift is reduced by 5 bits).
			 * TODO: decrease weight with rotation rate?
			 * TODO: rounding?
			 */
			static uint16_t prev_acc_angle = 0;
			static uint16_t prev_gyro_angle = 0;

			int16_t gyro_velo = prev_gyro_angle - angle;
			int16_t acc_velo = prev_acc_angle - acc_angle;
			prev_gyro_angle = angle;
			prev_acc_angle = acc_angle;

			if (correct && !(((uint16_t) acc_velo ^ gyro_velo) &
						0x8000) &&
					angle_accum < DEGS_TO_ANGLE(150.0f))
				config.gyro_mult += (((int32_t) abs(acc_velo) -
							abs(gyro_velo)) <<
						(14 - 7)) / angle_accum;

			if (config.gyro_mult < 0x2000)
				config.gyro_mult = 0x2000;
			if (config.gyro_mult > 0xc000)
				config.gyro_mult = 0xc000;

			//eeprom_save();
			angle_accum = 0;
		}
	}

	return angle;
}

//---------------------------------------------------------------------------------------
int main ()
{
/*Config*/
config.prog = 0;
config.gyro_mult = 1 << 14;
config.cf_acc[0] = 50.0f * 65536 * 16;
config.cf_acc[1] = -150.0f * 65536 * 16;
config.cf_samples = 0;

temps = millis();
i=1;
wiringPiSetup () ;
fd=wiringPiI2CSetup (adresse) ;
if (fd==-1)
{
//qDebug("Le port I2C n'est pas joignable. V  rifiez les pram  tres et connexions.");
printf("Le port I2C n'est pas joignable. V  rifiez les pram  tres et connexions.\n");
return -1;
}
else
{
wiringPiI2CWriteReg8(fd,0x6B,0x00);//r  veille le capteur en g  rant le power_mgmt

//master enable
wiringPiI2CWriteReg8(fd,0x6A,0x20);

//bypass enabled
wiringPiI2CWriteReg8(fd,0x37,0x20);

//set  scale gyro 2000 fs
wiringPiI2CWriteReg8(fd,0x1B,0x18);

//set scale accel 
wiringPiI2CWriteReg8(fd,0x1C,0x18);

// lireTous();
//initAngles();//r  cup  re les valeurs de repos
delay(5);
zero_gyro();

/* Accelerometer-based rough initial angle */
acc_update();
angle = atan2(acc[0], acc[1]) * (-32768.0f / M_PI);

/*todo start timer*/
clock_init();

while (1)
{
clock_refreshTimer();
/*getAngles();
printf("AngleX =%f, AngleY =%f, AngleZ=%f \n",angleX-angleX0,angleY-angleY0,angleZ-angleZ0);
delay(10);*/
gyro_update();
angle_update();

printf(" %i \n",angle);
}
}
return 0;
}

