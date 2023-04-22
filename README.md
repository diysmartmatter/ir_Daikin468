# ir_Daikin468
IR remote control program using IRremoteESP8266. https://github.com/crankyoldgit/IRremoteESP8266

The target of this library is Daikin A/C that uses Daikin ARC468A3 remote controller. 

![ARC468](https://diysmartmatter.com/wp-content/uploads/2023/02/daikinremo-scaled.jpg "ARC468")

You could buid a smart IR remote controller using ESP32:

![ESP32](https://diysmartmatter.com/wp-content/uploads/2023/02/pcb.jpg)

![Diagram](https://diysmartmatter.com/images/20221225150838.png)

that can be used by Apple HomeKit as a Heater/Cooler accessory. 

![HomeKit](https://diysmartmatter.com/images/20221123195125.png)

# Background

The IRremoteESP8266 library has a very excellent class for Daikin air-conditioners named IRDaikin2, although some of data bits in the IR data slightly differ from my Daikin products using an ARC468A3 remote controller. To fix the IR data, this class (IRremoteESP8266) has been created based on the IRDaikin2 class. Note that only essential methods required by HomeKit Heater Cooler Accessory are implemented. 

# Prerequisite

![Configuration](https://diysmartmatter.com/wp-content/uploads/2023/04/setup_E.jpg)

- Homebridge
- MQTT (Mosquitto)
- Arduino IDE
- ESP32 with IR LED(s)
- ESP32 Arduino (Board manager)
- Arduino libraries:
- IRremoteESP8266 
- ArduinoOTA
- EspMQTTClient
- DHT20


# How to use

- Put IRremoteESP8266.cpp and IRremoteESP8266.h in your IRremoteESP3266 source directry, such as ~/Documents/Arduino/libraries/IRremoteESP8266/src/
- Create an Arduino sketch using example.ino. (WiFi info and address should be changed to yours)
