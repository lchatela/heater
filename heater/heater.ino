#include <SPI.h>
#include <Ethernet.h>
#include <ThingSpeak.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>



float Voltage = 0.0;
#define PRINT_DEBUG_MESSAGES //thingspeak debug
#define DEBUG 1         //DEBUG=1 works, DEBUG=0 works now!
#define ANALOG_READ 1
#define CHECK_ALERT 1
#define CHECK_RELAY 1
#define READ_RS485 0
#define ETHERNET_ON 1


Adafruit_ADS1115 ads(0x48);



// constants won't change. They're used here to set pin numbers:
const int buttonPin = 45;// the number of the pushbutton pin
const int enableAnalogPin = 43; //to avoid current drain when arduino is off
const int ledPin =  13;      // the number of the LED pin
const int relayPompePin = 2; // pin to stop the pump relay
const int relayProjectorPin = 3; //pin to activate the projector

long currentMillis = 0;

#if CHECK_ALERT == 1
// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status
int nbPress = 0;
int lastPressCount = 0;
long previousMillis = 0;

long lastSentEmail = 0;

bool errorDetected = false;
bool emailSent = false;
bool firstLoop = true;



char server[] = "mail.smtp2go.com";
int port = 2525;
const unsigned long emailInterval = 3600L * 1000L;
#endif
int errorType = 0;
EthernetClient client;
byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x59, 0x67 };
byte ip[] = { 192, 168, 0, 66 };
byte google[] = { 64, 233, 187, 99 }; // Google

#if ANALOG_READ == 1
//ThingSpeak setup
char jsonBuffer[500] = "["; // Initialize the jsonBuffer to hold data
char serverIOT[] = "api.thingspeak.com";
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
float factor[] = {1.0, 1.0, 0.117, 1.0, 1.0, 0.129, 0.1265, 0.1245, 0.3171, 0.1241, 0.03, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,};
float offset[] = {0.0, 0.0, 149.3, 0.0, 0.0, 169.15, 164.87, 161.2, 370.29, 160.36, -1.8, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,};

int16_t adc0 = 0;
int stat = 0;
adsGain_t gain = GAIN_TWO;
long measure = 0;
const int NB_CYCLE = 20;
long lastAnalogMillis = 0;
const unsigned long analogDelay = 60L * 1000L;


# endif //end ANALOG_READ

#if CHECK_RELAY == 1
long lastRelayMillis = 0;
const unsigned long relayDelay = 20L * 1000L;
# endif //end check relay

#define Pin13LED         13

#if READ_RS485 == 1
#include <SoftwareSerial.h>
#include <RS485_heating.h>
#define SSerialRX        42  //Serial Receive pin
#define SSerialTX        48  //Serial Transmit pin

#define SSerialTxControl 44   //RS485 Direction control
#define SSerialTxControl2 46
#define RS485Transmit    HIGH
#define RS485Receive     LOW





//SoftwareSerial rs485 (SSerialRX, SSerialTX);

/*-----( Declare Variables )-----*/
int byteReceived;
int byteSend;


int fAvailable ()
{
  return rs485.available ();
}

int fRead ()
{
  return rs485.read ();
}


RS485 myChannel (fRead, fAvailable, NULL, 250);
#endif
#define relayPin1         44
void setup() {
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(Pin13LED, OUTPUT);
  pinMode(relayPompePin, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  digitalWrite(relayPompePin, HIGH);
  digitalWrite(3, HIGH);
  digitalWrite(4, HIGH);
  digitalWrite(5, HIGH);
  digitalWrite(6, HIGH);
  digitalWrite(7, HIGH);


  Serial.begin(9600);
#if ETHERNET_ON == 1
  Serial.println("Ethernet start");

  if (Ethernet.begin(mac) == 0 ) {
    Serial.println("Connection failed");
  }
  //Ethernet.begin(mac, ip);
  delay(1000);

  Serial.println("connecting...");



  Serial.println("ThingSpeak start");
  if (ThingSpeak.begin(client)) {
    Serial.println("OK");
  } else {

    Serial.println("nOK");
  }


#endif




  //setup multiplexer
  DDRA = B11111111;
  ads.begin();
  ads.setGain(gain);

#if READ_RS485 == 1
  //pinMode(SSerialTxControl, OUTPUT);
  //pinMode(SSerialTxControl2, OUTPUT);
  //rs485.begin(19200);


  //digitalWrite(SSerialTxControl, RS485Receive);  // Init Transceiver
  //digitalWrite(SSerialTxControl2, RS485Receive);
  // Start the software serial port, to another device
  //myChannel.begin ();

  Serial.println("RS started");
#endif

}


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
      muxValues[i] = Voltage * factor[i] - offset[i]; // read the vlaue on that pin and store in array
      delay(10); // let the mux switch
    }


    // display captured data
    displayData();

    digitalWrite(enableAnalogPin, LOW);
    lastAnalogMillis = currentMillis;
    if (currentMillis - lastThingSpeakUpdateTime >=  thingSpeakUpdateInterval) {
      Serial.println("writeThingSpeak");
      writeThingSpeak();


    }
  }
#endif
}

void checkAlertStatus() {
#if CHECK_ALERT == 1
  buttonState = digitalRead(buttonPin);


  // if nothing detected for one min
  if (currentMillis - previousMillis > 60000) {
    // if there was an error send stop email
    if (errorDetected == true) {
      Serial.println(F("error stopped -- email"));
      errorDetected = false;
      errorType = 0;
      nbPress = 0;
      sendEmail("Status ok", 0);
      emailSent = false;
      digitalWrite(ledPin, LOW);
    }

  }




  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == LOW) {

    errorDetected = true;
    nbPress = 1;
    Serial.println("error detected");


    long startErrorCount = millis();
    Serial.println(startErrorCount);

    delay(800);
    //start looping to count nbPress
    for (int j = 0; j < 15; j++) {
      buttonState = digitalRead(buttonPin);
      if (buttonState == LOW) {
        nbPress++;
        Serial.println("one more press");
      }
      //break if we are monitoring for more than 2 secs
      //or max error number is 3
      if (millis() - startErrorCount > 3000 || nbPress == 3) {
        Serial.println("breaking");
        break;
      }
      delay(800);

    }


    errorType = nbPress;
    Serial.print(F("Error Type "));
    Serial.println(errorType);
    nbPress = 0;
    if (!(emailSent) || (currentMillis - lastSentEmail > emailInterval)) {
      Serial.println(F("Sending email"));
      sendEmail("New error detected", errorType);
      emailSent = true;
      lastSentEmail = currentMillis;
    }

    previousMillis = currentMillis;

    // turn LED on:
    digitalWrite(ledPin, HIGH);

  }
#endif // was CHECK_ALERT == 1
}



void checkRelay() {
#if CHECK_RELAY == 1

  if (currentMillis - lastRelayMillis > relayDelay) {
    int pumpValue = ThingSpeak.readIntField(511114, 1, "DE8V7G7WCCDALMC9");
    int projectorValue = ThingSpeak.readIntField(511114, 2, "DE8V7G7WCCDALMC9");

    if (pumpValue == 1) {
      digitalWrite(relayPompePin, HIGH);
      Serial.println("pompe up");
    }
    else if (pumpValue == 0) {
      digitalWrite(relayPompePin, LOW);
      Serial.println("pompe down");

    }
    else {
      Serial.print("pompe unknown value");
      Serial.println(pumpValue);
    }

    if (projectorValue == 1) {
      digitalWrite(relayProjectorPin, LOW);
      Serial.println("projector up");
    }
    else if (projectorValue == 0) {
      digitalWrite(relayProjectorPin, HIGH);
      Serial.println("projector down");

    }
    else
    {
      Serial.print("pompe unknown value");
      Serial.println(pumpValue);
    }
    lastRelayMillis = currentMillis;
  }

#endif
}

void readRS485() {
#if READ_AS485 == 1
  if (myChannel.update ())
  {
    Serial.write (myChannel.getData (), myChannel.getLength ());
    Serial.println ();
  }

  //Copy input data to output

  digitalWrite(Pin13LED, HIGH);  // Show activity
  delay(10);
  digitalWrite(Pin13LED, LOW);


  // digitalWrite(SSerialTxControl, RS485Receive);  // Disable RS485 Transmit


#endif
}

void loop() {

  currentMillis = millis();

  readAnalog();
  checkAlertStatus();
  checkRelay();
  readRS485();
  delay(10);



  // read the state of the pushbutton value:





}
#if CHECK_ALERT
byte sendEmail(String subject, int errType)
{
  byte thisByte = 0;
  byte respCode;

  if (client.connect(server, port) == 1) {
    Serial.println(F("connected"));
  } else {
    Serial.println(F("connection failed"));
    return 0;
  }

  if (!eRcv()) return 0;

  Serial.println(F("Sending hello"));
  //// replace 1.2.3.4 with your Arduino's ip
  client.println(F("EHLO 1.2.3.4"));
  if (!eRcv()) return 0;

  Serial.println(F("Sending auth login"));
  client.println(F("auth login"));
  if (!eRcv()) return 0;

  Serial.println(F("Sending User"));
  //// Change to your base64 encoded user
  client.println(F("bGNoYXRlbGFAaG90bWFpbC5jb20="));
  //
  if (!eRcv()) return 0;
  //
  Serial.println(F("Sending Password"));
  //// change to your base64 encoded password
  client.println(F("Z3RoTlE5N0o="));
  //
  if (!eRcv()) return 0;
  //
  //// change to your email address (sender)
  Serial.println(F("Sending From"));
  client.println(F("MAIL From: <machaudiere@okofen.fr>"));
  if (!eRcv()) return 0;
  //
  //// change to recipient address
  Serial.println(F("Sending To"));
  client.println(F("RCPT To: <laurent.chatelain@gmail.com>"));
  if (!eRcv()) return 0;
  //
  Serial.println(F("Sending DATA"));
  client.println(F("DATA"));
  if (!eRcv()) return 0;
  //
  Serial.println(F("Sending email"));
  //
  // change to recipient address
  client.println(F("To: You <laurent.chatelain@gmail.com>"));
  //
  // change to your address
  client.println(F("From: Chaudiere <chaudiere@okofen.fr>"));
  //
  client.print(F("Subject: "));
  client.print(subject);
  client.println("\r\n");

  if (errType == 0) {
    client.println(F("Error stopped. All ok"));
  } else {
    client.print(F("Error detected. Type "));
    client.println(errType);
  }
  client.println(F("."));
  //
  if (!eRcv()) return 0;

  Serial.println(F("Sending QUIT"));
  client.println(F("QUIT"));
  if (!eRcv()) return 0;
  //
  client.stop();
  //
  Serial.println(F("disconnected"));
  //
  return 1;
}

byte eRcv()
{
  byte respCode;
  byte thisByte;
  int loopCount = 0;

  while (!client.available()) {
    delay(1);
    loopCount++;

    // if nothing received for 10 seconds, timeout
    if (loopCount > 10000) {
      client.stop();
      Serial.println(F("\r\nTimeout"));
      return 0;
    }
  }

  respCode = client.peek();

  while (client.available())
  {
    thisByte = client.read();
    Serial.write(thisByte);
  }

  if (respCode >= '4')
  {
    efail();
    return 0;
  }

  return 1;
}


void efail()
{
  byte thisByte = 0;
  int loopCount = 0;

  client.println(F("QUIT"));

  while (!client.available()) {
    delay(1);
    loopCount++;

    // if nothing received for 10 seconds, timeout
    if (loopCount > 10000) {
      client.stop();
      Serial.println(F("\r\nTimeout"));
      return;
    }
  }

  while (client.available())
  {
    thisByte = client.read();
    Serial.write(thisByte);
  }

  client.stop();

  Serial.println(F("disconnected"));
}
#endif

#if ANALOG_READ == 1
void writeThingSpeak() {

  Serial.println("connecting...");


  ThingSpeak.setField(1, errorType);
  ThingSpeak.setField(2, muxValues[2]);
  ThingSpeak.setField(3, muxValues[5]);
  ThingSpeak.setField(4, muxValues[6]);
  ThingSpeak.setField(5, muxValues[7]);
  ThingSpeak.setField(6, muxValues[8]);
  ThingSpeak.setField(7, muxValues[9]);
  ThingSpeak.setField(8, muxValues[10]);

#if DEBUG == 1
  Serial.println("write to thing speak values ");

  Serial.print(errorType);
  Serial.print(" ");
  for (int i = 0; i < 16; i++)
  {
    Serial.print(" ");
    Serial.print(muxValues[i]);
  }

  Serial.println(muxValues[0]);

#endif

  stat = ThingSpeak.writeFields(483551, "DZGL7BX9DA1I54MN");

  Serial.println("done write to thing speak");
  Serial.println(stat);
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
        Serial.print("chaudiere");
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

