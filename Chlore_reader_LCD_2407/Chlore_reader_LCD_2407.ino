#include "U8glib.h"
#include <SoftwareSerial.h>
#include <stdlib.h>
#include <RS485_non_blocking.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <OneWire.h>



const byte ENABLE_PIN = 7;
const byte RENABLE_PIN = 6;
const byte CL_RELAY_PIN = 9;
const byte CL_ALARM_PIN = 10;
const byte PH_RELAY_PIN = 11;
const byte PH_ALARM_PIN = 12;
const byte ONE_WIRE_PIN = 4;
const byte RAIN_PIN = 3;
const byte LED_PIN = 13;
boolean ack = false;
char  buf[50];
Adafruit_ADS1115 ads(0x48);


// DS18S20 Temperature chip i/o
OneWire ds(ONE_WIRE_PIN); // on pin 10


byte addr1[8] = {0x28, 0x56, 0x0A, 0x43, 0x98, 0x08, 0x00, 0x2C};
byte addr2[8] = {0x28, 0xA4, 0x11, 0x43, 0x98, 0x25, 0x00, 0x74};
int SignBit = 0;
char * sensorSummary;
//receiver, sender
SoftwareSerial rs485(8, 5);

struct  {
  float ORPValue;
  float PHValue;
  float RainValue;
  float WaterTemp;
  float AirTemp;
  boolean ORPAlarm;
  boolean PHAlarm;



} busMsg;

int fAvailable()
{
  return rs485.available();
}

int fRead()
{
  return rs485.read();
}

size_t fWrite (const byte what)
{
  // Serial.println(what);
  return rs485.write (what);
}

RS485 myChannel (fRead, fAvailable, fWrite, 60);
//22414 1172
float measure[3] = {0, 0};
int16_t adc[3] = {0, 0};
float ADC_Value[3] = {0, 0};

float sensorValue[3] = {0, 0};
//ORPValue = 0;
//float PHValue = 0;
const byte msgTrigger [] = "gogogo";
//number of measure0
int NB_CYCLE = 20;

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);  // I2C / TWI

boolean firstLoop = true;
const long delayInjection = 1800000;
long injStart[2] = {delayInjection * -1, delayInjection * -1};
long injTime[2] = {0, 0};
boolean injReq[2] = {false, false};
boolean injInProgress[2] = {false, false};
char injTimeStr[2][3];
byte injRelay[2] = {CL_RELAY_PIN, PH_RELAY_PIN};

char instructions[30];
char message[30];
char msgLine1[30] = "";
char msgLine2[30] = "";
char msgLine3[30] = "";
byte alarmPin[2] = {CL_ALARM_PIN, PH_ALARM_PIN};
boolean alarm[2] = {false,false};
char alarmStr[2][2] = {"0", "0"};
boolean state;


void drawAll() {
  u8g.setFont(u8g_font_unifont);
  u8g.drawStr( 0, 20, msgLine1);
  u8g.drawStr( 0, 40, msgLine2);
  u8g.drawStr( 0, 60, msgLine3);
}


void setup(void) {

  // assign default color value
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);     // white
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);         // max intensity
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);         // pixel on
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255, 255, 255);
  }

  Serial.begin(115200);
  rs485.begin (9600);
  pinMode (ENABLE_PIN, OUTPUT);  // driver output enable
  pinMode (RENABLE_PIN, OUTPUT);
  pinMode (CL_RELAY_PIN, OUTPUT);
  pinMode (PH_RELAY_PIN, OUTPUT);

  pinMode (CL_ALARM_PIN, INPUT_PULLUP);
  pinMode (PH_ALARM_PIN, INPUT_PULLUP);
  pinMode (RAIN_PIN, INPUT);
  pinMode (LED_PIN, OUTPUT);
  myChannel.begin();
  ads.begin();
  // char * buf = (char*) malloc(50);

}

void(* resetFunc) (void) = 0;


void loop(void) {



  if (firstLoop) {
    u8g.firstPage();
    strcpy(msgLine1, "Starting");
    do {
      drawAll();
    } while ( u8g.nextPage() );
    delay(1000);
    firstLoop = false;

  }


  strcpy(msgLine1, "Sensor reading");
  u8g.firstPage();
  do {
    drawAll();
  } while ( u8g.nextPage() );
  delay(100);
  sensorSummary = readSensor();



  //  char ln1[16];
  //  strncpy(ln1, sensorSummary, 16);
  //  ln1[16] = '\0';
  //  char ln2[16];
  //  strncpy(ln2, sensorSummary + 16, 16);
  //  ln2[16] = '\0';
  //
  //
  //  strcpy(msgLine1, ln1);
  //  strcpy(msgLine2, ln2);
  //  u8g.firstPage();
  //  do {
  //    drawAll();
  //  } while ( u8g.nextPage() );

  digitalWrite (ENABLE_PIN, LOW);
  digitalWrite (RENABLE_PIN, LOW);
  // wait to be called
  long waitingStart = millis();
  checkInjection();

  // wait 30 seconds max, then redoing the measure0
  while (millis() - waitingStart < 30000) {
    checkInjection();
    if (myChannel.update())
    {

      for (int i = 0; i < myChannel.getLength(); i++) {
        instructions[i] = (char)myChannel.getData()[i];
      }
      instructions[myChannel.getLength()] = '\0';

      memset (&msgLine1, '\0', 20);
      sprintf(msgLine1, "Recv [%s]", instructions);
      //strcpy(msgLine1, instructions);
      u8g.firstPage();
      do {
        drawAll;
      } while ( u8g.nextPage() );

      checkInstructions();
      checkInjection();

      digitalWrite (ENABLE_PIN, HIGH);
      digitalWrite (RENABLE_PIN, HIGH);
      delay(800);
      strcpy(msgLine1, "Sending");
      u8g.firstPage();
      do {
        drawAll;
      } while ( u8g.nextPage() );

      myChannel.sendMsg ((byte *) &busMsg, sizeof busMsg);
      strcpy(msgLine1, "Sent");
      u8g.firstPage();
      do {
        drawAll;
      } while ( u8g.nextPage() );

      break;


    }
  }

  strcpy(msgLine1, "Check injection");
  u8g.firstPage();
  do {
    drawAll;
  } while ( u8g.nextPage() );
  checkInjection();
  delay(100);
}


char * readSensor() {


  memset (&busMsg, 0, sizeof busMsg);
  memset (&buf, '\0', 50);



  for (int k = 0; k < 3; k++) {
    measure[k] = 0;
    ADC_Value[k] = 0;
    sensorValue[k] = 0;
  }
  // read the Chlore and PH value
  for (int j = 0; j < NB_CYCLE; j++) {
    for (int k = 0; k < 3; k++) {
      adc[k] = ads.readADC_SingleEnded(k);

      ADC_Value[k] = (adc[k] * 0.1875) / 1000;

      measure[k] += ADC_Value[k];


    }
    delay(50);
  }

  for (int k = 0; k < 3; k++) {
    ADC_Value[k] = (float)measure[k] / (float)NB_CYCLE;
    switch (k) {
      case 0:
        sensorValue[k] = ((2.5 - ADC_Value[k]) / 1.037) * 1000;
        busMsg.ORPValue = sensorValue[k];
        break;
      case 1:
        sensorValue[k] = -5.70 * ADC_Value[k] + 21.6;
        busMsg.PHValue = sensorValue[k];
        break;
      case 2:
        sensorValue[k] =  ADC_Value[k] * 1000;
        busMsg.RainValue = sensorValue[k];
        break;
      default:
        break;
    }

  }



  char sensorStr[3][10];

  dtostrf(sensorValue[0], 4, 1, sensorStr[0]);
  dtostrf(sensorValue[1], 2, 2, sensorStr[1]);
  dtostrf(sensorValue[2], 4, 0, sensorStr[2]);
  //check alarm
  for (int k = 0; k < 2; k++) {
    state = digitalRead(alarmPin[k]);
    if (state == LOW) {
      alarm[k] = true;
      strcpy( alarmStr[k],"1");
    } else {
      alarm[k] = false;
      strcpy(alarmStr[k],"0");
    }
  }

  busMsg.ORPAlarm = alarm[0];
  busMsg.PHAlarm = alarm[1];
  // ORP[-2000.1/1]{7.12/0}(32.45/12.4)
  // ORP[Chlorevalue/CloreAlarm]{PHValue/PHAlarm}(tempEau/tempAir)
  // keep ORP for compatibility with OpenHAb

  float temperature1;
  float temperature2;
  /* Lit la température ambiante à ~1Hz */
  getTemperature(addr1, &temperature1) ;
  getTemperature(addr2, &temperature2) ;

  busMsg.WaterTemp = temperature1;
  busMsg.AirTemp = temperature2;


  char temperatureStr[2][5];

  dtostrf(temperature1, 2, 1, temperatureStr[0]);
  dtostrf(temperature2, 2, 1, temperatureStr[1]);


  memset (&msgLine3, '\0', 20);
  memset (&msgLine2, '\0', 20);
  sprintf(msgLine2, "Cl[%s]Al{%s}Eau(%s)", sensorStr[0], alarmStr[0], temperatureStr[0]);
  sprintf(msgLine3, "Ph[%s]Al{%s}Air(%s)", sensorStr[1], alarmStr[1], temperatureStr[1]);
  // sprintf(buf, "ORP[%s-%s]{%s-%s}(%s-%s-%s)", sensorStr[0], alarmStr[0], sensorStr[1], alarmStr[1], temperatureStr[0], temperatureStr[1], sensorStr[2]);
  //sprintf(buf, "PH[%s]", sensorStr[0]);
  u8g.firstPage();
  do {
    drawAll();
  } while ( u8g.nextPage() );

  return buf;

}


void checkInjection() {
  strcpy(msgLine1, "Check injection start");
  long remaining = 0;
  u8g.firstPage();
  do {
    drawAll();
  } while ( u8g.nextPage() );

  for (int i = 0; i < 2; i++) {
    if (injInProgress[i]) {
      remaining = millis() - injStart[i];
      if ( remaining > injTime[i]) {
        digitalWrite (injRelay[i], LOW);
        digitalWrite (LED_PIN, LOW);
        injInProgress[i] = false;
        injReq[i] = false;

        if (i == 0) {
          strcpy(msgLine2, "Cl inj done");
        } else {
          strcpy(msgLine3, "Ph inj done");
        }


        //return;

      } else {
        if (i == 0) {
          memset(&msgLine2, '\0', 20);
          sprintf(msgLine2, "Cl progrs[%9ld]", remaining);
        } else {
          memset(&msgLine3, '\0', 20);
          sprintf(msgLine3, "Ph progrs[%9ld]", remaining);
        }
      }

    } else if (injReq[i]) {
      // do not redo an injection in the same 30 mins
      remaining = millis() - injStart[i];
      if ( remaining < delayInjection) {
        if (i == 0) {
          memset(&msgLine2, '\0', 20);
          sprintf(msgLine2, "Cl delay[%9ld]", remaining);
        } else {
          memset(&msgLine3, '\0', 20);
          sprintf(msgLine3, "Ph delay[%9ld]", remaining);
        }

        injReq[i] = false;


      } else  {
        digitalWrite (injRelay[i], HIGH);
        digitalWrite (LED_PIN, HIGH);
        injInProgress[i] = true;
        injStart[i] = millis();
        if (i == 0) {
          strcpy(msgLine2, "Cl injection Start");
        } else {
          strcpy(msgLine3, "Ph injection Start");
        }

      }

    }
  }
  u8g.firstPage();
  do {
    drawAll();
  } while ( u8g.nextPage() );
}



void checkInstructions() {
  //don't read during injection to avoid disturbance



  // injClTime = 0;
  strcpy(msgLine1, instructions);
  u8g.firstPage();
  do {
    drawAll();
  } while ( u8g.nextPage() );

  if (instructions[0] == 'R') {
    resetFunc();
  }
  if (instructions[0] == 'C' || instructions[0] == 'P') {
    int i = 3;
    switch (instructions[0])
    {
      case 'C':
        i = 0;
        break;

      case 'P':
        i = 1;
        break;

      default:
        break;
    }

    if (injInProgress[i]) {
      //should not happen, exit the check
      return;
    }
    injReq[i] = true;
    injTimeStr[i][0] = instructions[2];
    injTimeStr[i][1] = instructions[3];
    //  instructions.copy(injTimeStr[i],3,1);
    injTimeStr[i][2] = '\0';
    strcpy(msgLine3, injTimeStr[i]);
    u8g.firstPage();
    do {
      drawAll();
    } while ( u8g.nextPage() );



    injTime[i] = atol(injTimeStr[i]) * 1000 * 60 ;
    if (i == 0) {
      memset(&msgLine2, '\0', 20);
      sprintf(msgLine2, "Cl inj req[%i]", atoi(injTimeStr[i]));
    } else {
      memset(&msgLine3, '\0', 20);
      sprintf(msgLine3, "Ph inj req[%i]", atoi(injTimeStr[i]));
    }
    // sprintf(message, "inj time[%i]", atoi(injTimeStr[i]));

    //strcpy(msgLine2, message);
    u8g.firstPage();
    do {
      drawAll();
    } while ( u8g.nextPage() );
    delay(2000);


  } else {
    sprintf(msgLine1, "free ram[%i]", freeRam());
    // strcpy(msgLine3, "no request");
    u8g.firstPage();
    do {
      drawAll;
    } while ( u8g.nextPage() );
  }
}


int getTemperature(byte* addr, float *temperature) {
  int HighByte, LowByte, TReading;
  byte data[12];
  byte i;

  ds.reset();
  ds.select(addr);
  ds.write(0x44); // start conversion, with direct power

  delay(1000); // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.

  ds.reset();
  ds.select(addr);
  ds.write(0xBE); // Read Scratchpad

  for ( i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }



  /* Calcul de la température en degré Celsius */
  *temperature = (int16_t) ((data[1] << 8) | data[0]) * 0.0625;

}

int freeRam ()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

