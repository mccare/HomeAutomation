# Sensor Node on Rasperry Pi

## Temperature and humidty with DHT22/11

* Install https://github.com/adafruit/Adafruit_Python_DHT
* Configure the GPIO pin where your DHT22 is connected (and/or the type of your sensor)
* add the script to crontab to read out the sensor data

## Motion tracking with HC SR501

* change pir_motion.c to include your device_id and your PIN number
* 