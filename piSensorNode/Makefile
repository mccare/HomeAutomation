TARGETS = sn_motion_sensor sn_pwm sn_master_button sn_temperature

all: $(TARGETS)

install: all
	for target in $(TARGETS) ; do \
		if [ -e /usr/local/bin/$$target ] ; then \
			rm /usr/local/bin/$$target || true; \
		fi ; \
		cp $$target /usr/local/bin ; \
	done

sn_master_button: sn_master_button.c
	g++  sn_master_button.c -o sn_master_button  -lwiringPi

sn_motion_sensor : sn_motion_sensor.c
	g++  sn_motion_sensor.c -o sn_motion_sensor  -lwiringPi

sn_pwm : sn_pwm.c
	g++  sn_pwm.c -o sn_pwm  -lwiringPi

sn_temperature: ;
