


#include <RS485_non_blocking.h>
#include <SoftwareSerial.h>
const byte ENABLE_PIN = 7;
const byte RENABLE_PIN = 6;
const byte LED_PIN = 13;
//receiver, sender
SoftwareSerial rs485(8, 5);
boolean firstLoop = true;
const byte numChars = 32;
char receivedChars[numChars];
long startSending=-120000;

boolean newData = false;


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
  //Serial.println(what);
  return rs485.write (what);
}


RS485 myChannel(fRead, fAvailable, fWrite, 20);
const byte msgTrigger [20] = "Start";
long lastReceived = -100000;



void setup(void) {
  rs485.begin (9600);
  pinMode (ENABLE_PIN, OUTPUT);
  pinMode (RENABLE_PIN, OUTPUT);

  pinMode (LED_PIN, OUTPUT);
  Serial.begin(115200);
  myChannel.begin();
  Serial.println("starting");
}




void loop(void) {



  if (millis() - lastReceived > 10000) {
    if (millis()-startSending>120000){
      strcpy(msgTrigger,"Send");
    }
    recvWithStartEndMarkers();
    digitalWrite (LED_PIN, HIGH);
    delay(1000);
    digitalWrite (LED_PIN, LOW);
    delay(1000);
    digitalWrite (LED_PIN, HIGH);
    delay(1000);
    digitalWrite (LED_PIN, LOW);
    delay(1000);
    digitalWrite (ENABLE_PIN, HIGH);
    digitalWrite (RENABLE_PIN, HIGH);
    Serial.println("Ask update");
    myChannel.sendMsg (msgTrigger, sizeof (msgTrigger));


    delay(500);
    long startLoop = millis();
    digitalWrite (ENABLE_PIN, LOW);
    digitalWrite (RENABLE_PIN, LOW);
    Serial.println("wait answer");
    while (millis() - startLoop < 10000) {

      
      // wait to be called
      if (myChannel.update())
      {
        digitalWrite (LED_PIN, HIGH);
        delay(100);
        digitalWrite (LED_PIN, LOW);
        delay(100);
        digitalWrite (LED_PIN, HIGH);
        delay(100);
        digitalWrite (LED_PIN, LOW);
        delay(100);
        digitalWrite (LED_PIN, HIGH);
        delay(100);
        digitalWrite (LED_PIN, LOW);
        delay(100);
        //Serial.println("Slave Message reçu");
        Serial.write (myChannel.getData(), myChannel.getLength());
        Serial.println();
        lastReceived = millis();

        break;




      }







    }

    //recvWithStartEndMarkers();
    Serial.println("stop waiting answer");

  }

  // rebuild the picture after some delay
  //delay(2);
  //delay(5000);
}



// Example 3 - Receive with start- and end-markers




void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;
 
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
                strcpy(msgTrigger, receivedChars);
                Serial.print("received");
                //Serial.println(msgTrigger);
                Serial.println(receivedChars);
                newData = false;
                startSending=millis();
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}



