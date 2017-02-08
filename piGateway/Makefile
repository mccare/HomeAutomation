TARGETS= sn_gateway sn_sender_receiver

all: $(TARGETS)

install: all
	for target in $(TARGETS) ; do \
		if [ -e /usr/local/bin/$$target ] ; then \
			rm /usr/local/bin/$$target || true; \
		fi ; \
		cp $$target /usr/local/bin ; \
	done

sn_gateway : Gateway.c rfm69.cpp
	g++ Gateway.c rfm69.cpp -o sn_gateway -lwiringPi -lmosquitto -DRASPBERRY -DDAEMON

sn_sender_receiver : SenderReceiver.c rfm69.cpp transport.c
	g++ SenderReceiver.c rfm69.cpp transport.c -o sn_sender_receiver -lwiringPi -DRASPBERRY
