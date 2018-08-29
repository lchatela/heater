
#if ANALOG_READ == 1
//ThingSpeak setup
//char jsonBuffer[500] = "["; // Initialize the jsonBuffer to hold data
//char serverIOT[] = "api.thingspeak.com";
/* Collect data once every 15 seconds and post data to ThingSpeak channel once every 2 minutes */
unsigned long lastConnectionTime = 0; // Track the last connection time
unsigned long lastThingSpeakUpdateTime = 0; // Track the last update time
const unsigned long postingInterval = 30L * 1000L; // Post data every 2 minutes
const unsigned long thingSpeakUpdateInterval = 20L * 1000L; // Update once every 15 seconds



// control pins output table in array form
// see truth table on page 2 of TI 74HC4067 data sheet
// connect 74HC4067 S0~S3 to Arduino D7~D4 respectively
// connect 74HC4067 pin 1 to Arduino A0
byte controlPins[] = {B00000000,
                      B10000000,
                      B01000000,
                      B11000000,
                      B00100000,
                      B10100000,
                      B01100000,
                      B11100000,
                      B00010000,
                      B10010000,
                      B01010000,
                      B11010000,
                      B00110000,
                      B10110000,
                      B01110000,
                      B11110000
                     };

// holds incoming values from 74HC4067
float muxValues[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,};
float factor[] = {1.0, 1.0, 0.117, 1.0, 1.0, 0.129, 0.1265, 0.1245, 0.3171, 0.1241, 0.01,0.01, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,};
float offset[] = {0.0, 0.0, 149.3, 0.0, 0.0, 169.15, 164.87, 161.2, 370.29, 160.36, 2500, 2500, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,};
//factor 11 0.03 -1.8
int16_t adc0 = 0;
int stat = 0;
adsGain_t gain = GAIN_TWO;
long measure = 0;
const int NB_CYCLE = 20;
long lastAnalogMillis = 0;
const unsigned long analogDelay = 60L * 1000L;


# endif //end ANALOG_READ


#if ANALOG_READ == 1
void writeThingSpeak() {

  Serial.println("connecting...");


  //ThingSpeak.setField(1, errorType);
  MQTTsendFloat("heater/error",errorType);
  //ThingSpeak.setField(2, muxValues[2]);
  MQTTsendFloat("heater/temp/external",muxValues[2]);
  //ThingSpeak.setField(3, muxValues[5]);
  MQTTsendFloat("heater/temp/ecs",muxValues[5]);
  //ThingSpeak.setField(4, muxValues[6]);
  MQTTsendFloat("heater/temp/BTup",muxValues[6]);
  //ThingSpeak.setField(5, muxValues[7]);
  MQTTsendFloat("heater/temp/BTmed",muxValues[7]);
  //ThingSpeak.setField(6, muxValues[8]);
  MQTTsendFloat("heater/temp/solarPannel",muxValues[8]);
  //ThingSpeak.setField(7, muxValues[9]);
  MQTTsendFloat("heater/temp/solarAccu",muxValues[9]);
  //ThingSpeak.setField(8, muxValues[10]);
  //MQTTsendFloat("heater/temp/general",muxValues[10]); n'était pas raccordé
  MQTTsendFloat("pool/conso/pump",muxValues[10]);
  MQTTsendFloat("pool/conso/pac",muxValues[11]);
  // send ram information
  int ram=freeRam();
  logMessageAndInt("Ram=", ram, true);
  MQTTsendFloat("system/arduino1/ram",ram); 
#if DEBUG == 1
  Serial.println("Analog read debug");

  Serial.print(errorType);
  Serial.print(" ");
  for (int i = 0; i < 16; i++)
  {
    Serial.print(" ");
    Serial.print(muxValues[i]);
  }

  Serial.println(muxValues[0]);

#endif

  //stat = ThingSpeak.writeFields(483551, "DZGL7BX9DA1I54MN");

  //Serial.println("done write to thing speak");
  //logMessageAndInt("update values in thing speak ",stat, true);
  //Serial.println(stat);
  lastThingSpeakUpdateTime = millis();
}


void setPin(int outputPin)
// function to select pin on 74HC4067
{
  PORTA = controlPins[outputPin];
}



void displayData()
// dumps captured data from array to serial monitor
{
  Serial.println();
  Serial.println("Values from multiplexer:");
  Serial.println("========================");
  for (int i = 0; i < 16; i++)
  {

    switch (i) {
      case 0:
        Serial.print("circuit 1");
        break;
      case 1:
        Serial.print("circuit 2");
        break;
      case 2:
        Serial.print("exterieur");
        break;
      case 3:
        Serial.print("depart1");
        break;
      case 4:
        Serial.print("depart 2");
        break;
      case 5:
        Serial.print("ecs");
        break;
      case 6:
        Serial.print("bt haut");
        break;
      case 7:
        Serial.print("bt milieu");
        break;
      case 8:
        Serial.print("collecteur");
        break;
      case 9:
        Serial.print("accu 1");
        break;
      case 10:
        Serial.print("pompe piscine conso");
        break;
      case 11:
        Serial.print("pompe chaleur conso");
        break;
      
      default:
        Serial.print("input I");
        Serial.print(i + 1);
        // statements
    }

    Serial.print(" = ");
    Serial.println(muxValues[i]);
  }
  Serial.println("========================");
}
#endif

void readAnalog() {
#if ANALOG_READ == 1

  //check if analogRead should start (every 30 sec)
  if (currentMillis - lastAnalogMillis > analogDelay) {



    digitalWrite(enableAnalogPin, HIGH);
    delay(50); //let the IC start

    for (int i = 0; i < 16; i++)
    {
      setPin(i); // choose an input pin on the 74HC4067
      // we read from the ADC, we have a sixteen bit integer as a result

      measure = 0;

      for (int j = 0; j < NB_CYCLE; j++) {
        measure += ads.readADC_SingleEnded(0);
      }
      adc0 = measure / NB_CYCLE;
      //Serial.println(adc0);
      //Voltage = (adc0 * 0.1875) / 1000 * factor[i];
      Voltage = (adc0 * 0.0625);
      if (i<10) {
        muxValues[i] = Voltage * factor[i] - offset[i]; // read the vlaue on that pin and store in array
      } else
      {
        muxValues[i] = (Voltage -offset[i])* factor[i] ;
        
      }
      delay(10); // let the mux switch
    }


    // display captured data
    displayData();
    

    digitalWrite(enableAnalogPin, LOW);
    lastAnalogMillis = currentMillis;
    if (currentMillis - lastThingSpeakUpdateTime >=  thingSpeakUpdateInterval) {
      Serial.println("updating analog values");
      writeThingSpeak();


    }
  }
#endif
}


int freeRam ()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}


