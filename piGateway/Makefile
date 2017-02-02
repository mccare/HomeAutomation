install : Gatewayd
	cp Gatewayd /usr/local/bin
	cp GatewaydService /etc/init.d/Gatewayd
	chmod +x /etc/init.d/Gatewayd
	update-rc.d Gatewayd defaults
	service Gatewayd start

install_systemd: Gateway
	systemctl stop sn_gateway || true
	cp Gateway /usr/local/bin
	cp sn_gateway.service /etc/systemd/system
	systemctl daemon-reload
	systemctl start sn_gateway
	systemctl enable sn_gateway

uninstall:
	service Gatewayd stop
	update-rc.d -f Gatewayd remove
	rm /etc/init.d/Gatewayd
	rm /usr/local/bin/Gatewayd

Gatewayd : Gateway.c rfm69.cpp
	g++ Gateway.c rfm69.cpp -o Gatewayd -lwiringPi -lmosquitto -DRASPBERRY -DDAEMON

Gateway : Gateway.c rfm69.cpp
	g++ Gateway.c rfm69.cpp -o Gateway -lwiringPi -lmosquitto -DRASPBERRY -DDEBUG

SenderReceiver : SenderReceiver.c rfm69.cpp transport.c
	g++ SenderReceiver.c rfm69.cpp transport.c -o SenderReceiver -lwiringPi -DRASPBERRY
