/* IRremoteESP8266 and Daikin468 over MQTT */

// This is an Arduino sketch to control Daikin A/C through IR remote signal,
// using ESP8266, EspMQTTClient, and Arduino OTA.
// This sketch is supposed to be used with Homebridge and MQTTThing plugin.
// The example config part for the Homebridge is:
// 
// {
//     "type": "heaterCooler",
//     "name": "Living Aircon",
//     "url": "mqtt://localhost:1883",
//     "topics": {
//         "setActive": "mqttthing/irLiving/set/Active",
//         "setCoolingThresholdTemperature": "mqttthing/irLiving/set/CoolingThresholdTemperature",
//         "getCurrentTemperature": "mqttthing/irLiving/get$.temperature",
//         "setHeatingThresholdTemperature": "mqttthing/irLiving/set/HeatingThresholdTemperature",
//         "setRotationSpeed": "mqttthing/irLiving/set/RotationSpeed",
//         "setSwingMode": "mqttthing/irLiving/set/SwingMode",
//         "setTargetHeaterCoolerState": "mqttthing/irLiving/set/TargetHeaterCoolerState"
//     },
//     "restrictHeaterCoolerState": [
//         1,
//         2
//     ],
//     "accessory": "mqttthing"
// },

#include <Arduino.h>
#include <ArduinoOTA.h>

//IR Remote
#include "ir_Daikin468.h"
const uint16_t kIrLed = 4;  // ESP8266 GPIO pin to use. Recommended: 4 (D2).
IRDaikin468 daikin=IRDaikin468(kIrLed);
uint8_t _CoolingThresholdTemperature = 28; //HK has specific temp-value for each mode 
uint8_t _HeatingThresholdTemperature = 22; //HK has specific temp-value for each mode

//DHT sensor
#include "DHT20.h"
#define GPIO_SDA 21 //I2C for DHT20
#define GPIO_SCL 22 //I2C for DHT20
DHT20 dht; //instance of DHT20
//MQTT
#include <EspMQTTClient.h>
EspMQTTClient *client; //instance of MQTT client

//WiFi & MQTT
const char SSID[] = "xxxxxxxx"; //WiFi SSID
const char PASS[] = "XXXXXXXX"; //WiFi password
char CLIENTID[] = "IRremote_7497621"; //something random
const char  MQTTADD[] = "192.168.99.99"; //Broker IP address
const short MQTTPORT = 1883; //Broker port
const char  MQTTUSER[] = "";//Can be omitted if not needed
const char  MQTTPASS[] = "";//Can be omitted if not needed
const char  SUBTOPIC[] = "mqttthing/irLiving/set/#"; //mqtt topic to subscribe
const char  PUBTOPIC[] = "mqttthing/irLiving/get"; //to publish temperature
const char  DEBUG[] = "mqttthing/irLiving/debug"; //topic for debug

void setup() {
  dht.begin(GPIO_SDA, GPIO_SCL); //DHT20
  daikin.begin();
  client = new EspMQTTClient(SSID,PASS,MQTTADD,MQTTUSER,MQTTPASS,CLIENTID,MQTTPORT); 
  delay(1000);
}

void onMessageReceived(const String& topic, const String& message) { 
  String command = topic.substring(topic.lastIndexOf("/") + 1);

  if (command.equals("Active")) {
    if(message.equalsIgnoreCase("true")) daikin.on();
    if(message.equalsIgnoreCase("false")) daikin.off();
    daikin.send(0);
    client->publish(DEBUG,daikin.toChars());
  }else if(command.equals("TargetHeaterCoolerState")){
    if(message.equalsIgnoreCase("COOL")) {
      daikin.setTemp(_CoolingThresholdTemperature);//each mode has specific temp
      daikin.setMode(kDaikinCool);
    }
    if(message.equalsIgnoreCase("HEAT")) {
      daikin.setTemp(_HeatingThresholdTemperature);//each mode has specific temp
      daikin.setMode(kDaikinHeat);
    }
  }else if(command.equals("CoolingThresholdTemperature")){
    daikin.setTemp(message.toInt());
    _CoolingThresholdTemperature=daikin.getTemp(); //may be capped
    daikin.send(0);
    client->publish(DEBUG,daikin.toChars());
  }else if(command.equals("HeatingThresholdTemperature")){
    daikin.setTemp(message.toInt());
    _HeatingThresholdTemperature=daikin.getTemp(); //may be capped
    daikin.send(0);
    client->publish(DEBUG,daikin.toChars());
  }else if(command.equals("SwingMode")){
    if(message.equalsIgnoreCase("DISABLED")) daikin.setSwingVertical(kDaikinSwingOff);
    if(message.equalsIgnoreCase("ENABLED")) daikin.setSwingVertical(kDaikinSwingOn);
    daikin.send(0);
    client->publish(DEBUG,daikin.toChars());
  }else if(command.equals("RotationSpeed")){
    int speed = message.toInt();
    if     (speed < 20) daikin.setFan(kDaikin468FanAuto); //auto
    else if(speed < 40) daikin.setFan(kDaikin468FanQuiet); //sizuka
    else if(speed < 50) daikin.setFan(kDaikin468Fan1); //level-1
    else if(speed < 60) daikin.setFan(kDaikin468Fan2); //level-2
    else if(speed < 70) daikin.setFan(kDaikin468Fan3); //level-3
    else if(speed < 80) daikin.setFan(kDaikin468Fan4); //level-4
    else                daikin.setFan(kDaikin468Fan5); //level-5 max
   }
}

void onConnectionEstablished() {
  ArduinoOTA.setHostname("irLiving");
  ArduinoOTA.setPasswordHash("99999999999999999");
  ArduinoOTA.begin();
  Serial.println("MQTT connection established.");
  client->subscribe(SUBTOPIC, onMessageReceived); //set callback function
  client->publish(DEBUG,"irOffice started.");
}

//IR Remo and MQTT: read DHT20 and publish results
//check every 10 sec.
//if temp or humi change publish soon, otherwise publish every 5 min.
void publishDHT() { 
  char buff[64];
  static int count10=0; //counts up in every 10 sec.
  float humi, temp;
  static int oldhumi=0, oldtemp=0; //previous value
  int newhumi, newtemp; //new value
  if(millis() - dht.lastRead() < 10000) return;//do nothing < 10 sec.
  count10++; //count up 10 s counter
  if(DHT20_OK != dht.read()){
    client->publish(DEBUG,"DHT20 Read Error.");
    return; //sensor error, do nothing.
  }
  //read the current temp and humi
  humi=dht.getHumidity();
  newhumi=round(humi);//int version
  temp=dht.getTemperature();
  newtemp=round(temp * 10);//int version (x10)
  //if measurement changes or 300 seconds passed
  if((oldtemp != newtemp) || (oldhumi != newhumi) || (count10 >= 30)){
    oldtemp=newtemp;
    oldhumi=newhumi;    
    sprintf(buff, "{\"temperature\":%.1f,\"humidity\":%.0f}", temp, humi);
    client->publish(PUBTOPIC,buff);
    count10=0; //reset counter
  }
}

void loop() {
  ArduinoOTA.handle();
  client->loop(); 
  publishDHT(); //publish temp and humi if needed
}
