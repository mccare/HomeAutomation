# Sensor Node on Rasperry Pi

## Temperature and humidty with DHT22/11

* Install https://github.com/adafruit/Adafruit_Python_DHT
* Configure the GPIO pin where your DHT22 is connected (and/or the type of your sensor)
* add the script to crontab to read out the sensor data

## Motion tracking with HC SR501

* change pir_motion.c to include your device_id and your PIN number
* compile with make

# Installation

Configure the temperature reading via crontab:
```
* * * * * /path/to/HomeAutomation/piSensorNode/sn_temperature.py
```

Enable the SenderReceiver (for sending the RFM69 packages) and the motion tracker via systemd and change the path to the executable:
```
cp /path/to/sn_sender.service /etc/systemd/system
sudo systemctl enable  sn_sender.service 
cp /path/to/sn_motion_sensor.service /etc/systemd/system
sudo systemctl enable  sn_motion_sensor.service  
```

