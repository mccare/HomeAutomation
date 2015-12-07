#!/usr/bin/env python

import Adafruit_DHT
import stat, os, sys
import os.path

fifo = "/tmp/SenderReceiverIO.named_pipe"

# PIN GPIO 4
pin = 4

# RFM Device ID (will be then in the mosquitto topic)
device_id = 10

if not stat.S_ISFIFO(os.stat(fifo).st_mode) :
    print "FIFO at " + fifo + " not present"
    sys.exit(1)


humidity, temperature = Adafruit_DHT.read_retry(Adafruit_DHT.DHT22, pin)
with open(fifo, 'w+') as f:
    f.write( 'DEVICE_ID:{}:VALUE:{}'.format(device_id, temperature) )
    
with open(fifo, 'w+') as f:
    f.write( 'DEVICE_ID:{}:VALUE:{}'.format(device_id, humidity) )
    
#	print 'Temp={0:0.1f}*C  Humidity={1:0.1f}%'.format(temperature, humidity)

