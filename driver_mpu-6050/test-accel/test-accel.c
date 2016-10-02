#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "stdint.h"
#include "math.h"
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
#define MICROS() 0
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
	now = MICROS();  //Get systick
	timediff = now - prev;
	prev = now;
}



//---------------------------------------------------------------------------------------
int main ()
{
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
// lireTous();
initAngles();//r  cup  re les valeurs de repos
while (1)
{
getAngles();
//qDebug()<<"AngleX="<<angleX-angleX0<<" AngleY="<<angleY-angleY0<<endl;
//qDebug()<<"AngleX="<<angleX-angleX0<<" AngleY="<<angleY-angleY0<<endl;
printf("AngleX =%f, AngleY =%f, AngleZ=%f \n",angleX-angleX0,angleY-angleY0,angleZ-angleZ0);
delay(10);
}
}
return 0;
}

