Send Receive Test
=================

Below the original readme. I have changed the Gateway.c code so it doesn't talk to mosquitto any more and will just send and receive packets for testing out my RFM69CW config. Motivation being to debug my RFM69CW sensor setup.

On the first node compile with 
```
g++ Gateway.c rfm69.cpp -o Gateway -lwiringPi -lmosquitto -DRASPBERRY -DDEBUG
```

On the second node add:
```
g++ Gateway.c rfm69.cpp -o Gateway -lwiringPi -lmosquitto -DRASPBERRY -DDEBUG -DGATEWAY
```



Original Readme
===============



This repo contain a port of LowPowerLab RFM69 library, initially targeted at Arduino, to Raspberry Pi.
The Gateway code is a 1:1 replacement from Eric Tsai one to be run dirrectly on Raspberry, removing the need of a dedicated Arduino with Ethernet shield

Connect the RFM69 to the Raspberry PI

Using SPI part of Raspberry expantion port, with the IRQ connected to the pin 22 (GPIO_25)

```
3.3V  17
GND   20
SLCK  23
MISO  21
MOSI  19
NSS   24
DID0  22
```

The layout of the connection header is described at http://www.megaleecher.net/Raspberry_Pi_GPIO_Pinout_Helper
![Alt Text](http://www.megaleecher.net/sites/default/files/images/raspberry-pi-rev2-gpio-pinout.jpg "Raspberry Pinout")

Install Git core, if not already done
```
sudo apt-get install git-core
```
Download the WiringPi latest version
```
git clone git://git.drogon.net/wiringPi
```
A script is provided to easily build it
```
cd wiringPi
./build
```
:warning: Ensure you properly setup the SPI interface, using `raspi-config`


Install Mosquitto and the development libraries - based on http://mosquitto.org/2013/01/mosquitto-debian-repository

:warning: The repository doesn't seem able to deliver the proper libmosquitto-dev package, so we will compile it at a later stage.
```
wget http://repo.mosquitto.org/debian/mosquitto-repo.gpg.key
sudo apt-key add mosquitto-repo.gpg.key
cd /etc/apt/sources.list.d/
sudo wget http://repo.mosquitto.org/debian/mosquitto-wheezy.list
sudo apt-get update
sudo apt-get install mosquitto mosquitto-clients
```

Then, prepare the libmosquitto-dev replacement. Assume we will work from our home directory
```
cd ~
wget http://repo.mosquitto.org/debian/mosquitto-repo.gpg.key
sudo apt-key add mosquitto-repo.gpg.key
cd /etc/apt/sources.list.d/
sudo wget http://repo.mosquitto.org/debian/mosquitto-wheezy.list~
cd ~
sudo apt-get update
sudo apt-get install mosquitto mosquitto-clients
sudo apt-get install libssl-dev libwrap0-dev libc-ares-dev uuid-dev xsltproc docbook-xsl
wget http://mosquitto.org/files/source/mosquitto-1.3.5.tar.gz
tar zxvf mosquitto-1.3.5.tar.gz
cd mosquitto-1.3.5/lib/
make all
sudo make install
```

Grab the gateway
```
git clone git://github.com/abouillot/HomeAutomation
```
Compile the gateway
```
cd HomeAutomation/piGateway
g++ Gateway.c rfm69.cpp -o Gateway -lwiringPi -lmosquitto -DRASPBERRY -DDEBUG
```

You can omit the -DDEBUG part, if you don't want the debug output to be produced

Launch the gateway
```
sudo ./Gateway
```
sudo is required as some of the WiringPi library need it


### Daemon
The Gateway can also be run as a daemon

To build it you can use 
```
make Gatewayd
```

To intsall the Gateway as a service and run it
```
sudo make install
```
This will build it as well, if not already done. The service will be lauch at every startup of the system.

To remove the service you can use the command
```
sudo make uninstall
```
