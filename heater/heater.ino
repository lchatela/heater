#include <SPI.h>
#include <Ethernet.h>
#include <ThingSpeak.h>

//#define MQTT_VERSION 3
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>

float Voltage = 0.0;
#define PRINT_DEBUG_MESSAGES //thingspeak debug
#define DEBUG 1         //DEBUG=1 works, DEBUG=0 works now!
#define ANALOG_READ 1
#define CHECK_ALERT 1
#define CHECK_RELAY 0
#define READ_RS485 0
#define ETHERNET_ON 1
#define MQTT_ON 1





const int buttonPin = 45;// the number of the pushbutton pin
const int enableAnalogPin = 43; //to avoid current drain when arduino is off
const int ledPin =  13;      // the number of the LED pin
const int relayPompePin = 2; // pin to stop the pump relay
const int relayProjectorPin = 3; //pin to activate the projector

long currentMillis = 0;
int errorType = 0;
EthernetClient client;
EthernetClient ethernetMQTTclient;
PubSubClient MQTTclient(ethernetMQTTclient);
Adafruit_ADS1115 ads(0x48);

byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x59, 0x67 };
byte ip[] = { 192, 168, 0, 66 };
byte google[] = { 64, 233, 187, 99 }; // Google
bool firstLoop=true;


#define Pin13LED         13
#define relayPin1         44


