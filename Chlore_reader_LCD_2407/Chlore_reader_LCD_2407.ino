

#include "U8glib.h"
#include <SoftwareSerial.h>
#include <stdlib.h>
#include <RS485_non_blocking.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>

const byte ENABLE_PIN = 7;
const byte RENABLE_PIN = 6;
const byte RELAY_PIN = 9;
const byte CL_ALARM = 10;
const byte LED_PIN = 13;
boolean ack = false;
Adafruit_ADS1115 ads(0x48);

//receiver, sender
SoftwareSerial rs485(8, 5);

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

RS485 myChannel (fRead, fAvailable, fWrite, 20);

float measure = 0;
const byte msgTrigger [] = "gogogo";
//number of measure
int NB_CYCLE = 20;

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);  // I2C / TWI

boolean firstLoop = true;
const long delayInjection = 1800000;
long injectionStart = delayInjection*-1;
long injectionTime = 0;
boolean injectionRequested = false;
boolean injectionInProgress = false;
char injectionTimeStr [3];
char instructions[30];
char message[30];
char msgLine1[30] = "";
char msgLine2[30] = "";
char msgLine3[30] = "";


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
  pinMode (RELAY_PIN, OUTPUT);
  pinMode (LED_PIN, OUTPUT);
  myChannel.begin();
  ads.begin();

}




void loop(void) {
  // picture loop
  // Serial.println("Starting");
  int16_t adc0;


  if (firstLoop) {


    u8g.firstPage();
    strcpy(msgLine2, "Starting"); 
    do {
      drawAll();
    } while ( u8g.nextPage() );
    delay(2000);
    firstLoop = false;

  }


  measure = 0;
  float ADC_Value = 0;
  float PhidgetORP = 0;
  for (int j = 0; j < NB_CYCLE; j++) {
    adc0 = ads.readADC_SingleEnded(0);
    ADC_Value = (adc0 * 0.1875) / 1000;
    // ADC_Value = analogRead(A0) * 5 / 1023.0;
    delay(50);
    Serial.println(ADC_Value);
    measure += ADC_Value;
  }

  ADC_Value = (float)measure / (float)NB_CYCLE;


  PhidgetORP = ((2.5 - ADC_Value) / 1.037) * 1000;
  Serial.print("ORP measure= ");
  Serial.println(PhidgetORP);

  char myORPStr[10];
  dtostrf(PhidgetORP, 4, 3, myORPStr);
  char ORPMsg[20];
  sprintf(ORPMsg, "ORP[%s]", myORPStr);
  Serial.print("ORPMsg-");
  Serial.println(ORPMsg);

  //Serial.println(myORPStr);
  strcpy(msgLine2, "--");
  u8g.firstPage();
  do {
    drawAll();
  } while ( u8g.nextPage() );
  delay(100);


  strcpy(msgLine1, ORPMsg);
  u8g.firstPage();
  do {
    drawAll();
  } while ( u8g.nextPage() );

  digitalWrite (ENABLE_PIN, LOW);
  digitalWrite (RENABLE_PIN, LOW);
  // wait to be called
  long waitingStart = millis();
  checkInjection();
  Serial.println("waiting for instructions");
  // wait 30 seconds max, then redoing the measure
  while (millis() - waitingStart < 30000) {
    checkInjection();
    if (myChannel.update())
    {

      Serial.write (myChannel.getData(), myChannel.getLength());
      Serial.println();

      for (int i = 0; i < myChannel.getLength(); i++) {
        instructions[i] = (char)myChannel.getData()[i];
      }
      instructions[myChannel.getLength()] = '\0';

      strcpy(msgLine2, instructions);
      u8g.firstPage();
      do {
        drawAll;
      } while ( u8g.nextPage() );

      checkInstructions();
      checkInjection();

      digitalWrite (ENABLE_PIN, HIGH);
      digitalWrite (RENABLE_PIN, HIGH);
      delay(800);
      strcpy(msgLine2, "Sending");
      u8g.firstPage();
      do {
        drawAll;
      } while ( u8g.nextPage() );
      Serial.println("send value");

      myChannel.sendMsg (ORPMsg, sizeof (ORPMsg));
      break;


    }
  }

  checkInjection();
  delay(1000);
}


void checkInjection() {


  if (injectionInProgress) {
    if (millis() - injectionStart > injectionTime) {
      digitalWrite (RELAY_PIN, LOW);
      digitalWrite (LED_PIN, LOW);
      injectionInProgress = false;
      injectionRequested = false;
      strcpy(msgLine3, "Inj Finished");
      u8g.firstPage();
      do {
        drawAll();
      } while ( u8g.nextPage() );

      return;

    }
    sprintf(message, "progress[%15ld]", atol(millis() - injectionStart ) );
    strcpy(msgLine3, message);
    u8g.firstPage();
    do {
      drawAll();
    } while ( u8g.nextPage() );
    return;
  }

  if (injectionRequested) {
    // do not redo an injection in the same 30 mins
    if ( millis() - injectionStart < delayInjection) {
      sprintf(message, "wait[%15ld]", atol(millis() - injectionStart ) );
      strcpy(msgLine3, message);
      u8g.firstPage();
      do {
        drawAll;
      } while ( u8g.nextPage() );
      injectionRequested = false;


    } else  {
      digitalWrite (RELAY_PIN, HIGH);
      digitalWrite (LED_PIN, HIGH);
      injectionInProgress = true;
      injectionStart = millis();
      strcpy(msgLine3, "Injection Start");
      u8g.firstPage();
      do {
        drawAll();
      } while ( u8g.nextPage() );
    }

  }
}



void checkInstructions() {
  //don't read during injection to avoid disturbance
  
  if (injectionInProgress){
    return;
  }
  
 // injectionTime = 0;
  strcpy(msgLine2, instructions);
  u8g.firstPage();
  do {
    drawAll();
  } while ( u8g.nextPage() );

  if (instructions[0] == 'C') {
    injectionRequested = true;

    injectionTimeStr[0] = instructions[3];
    injectionTimeStr[1] = '\0';
    strcpy(msgLine2, injectionTimeStr);
    u8g.firstPage();
    do {
      drawAll();
    } while ( u8g.nextPage() );


    injectionTime = atol(injectionTimeStr) * 1000*60 ;
    sprintf(message, "inj time[%15ld]", injectionTime);
    strcpy(msgLine2, message);
    u8g.firstPage();
    do {
      drawAll();
    } while ( u8g.nextPage() );


  } else {
    strcpy(msgLine3, "no request");
      u8g.firstPage();
      do {
        drawAll;
      } while ( u8g.nextPage() );
  }
}


