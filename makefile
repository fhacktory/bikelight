bikelight: main.c screen/screen.c screen/frame.c mpu/mpu.c lib/tlc5947/tlc5947.c
	gcc -o bikelight main.c screen/screen.c screen/frame.c mpu/mpu.c lib/tlc5947/tlc5947.c -I. -lwiringPi -lm

test_rail_led: test/test_rail_led.c
	gcc -o test_rail_led test/test_rail_led.c -I. -lwiringPi 