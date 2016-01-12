all: sn_motion_sensor sn_pwm

sn_motion_sensor : sn_motion_sensor.c
	g++  sn_motion_sensor.c -o sn_motion_sensor  -lwiringPi 
	
sn_pwm : sn_pwm.c
	g++  sn_pwm.c -o sn_pwm  -lwiringPi 
