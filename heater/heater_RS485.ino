
#if READ_RS485 == 1
#include <SoftwareSerial.h>
#include <RS485_non_blocking.h>
#define SSerialRX        9//42  //Serial Receive pin
#define SSerialTX        8 //48 //Serial Transmit pin

#define SSerialTxControl 44   //RS485 Direction control
#define SSerialTxControl2 42 //46
#define RS485Transmit    HIGH
#define RS485Receive     LOW





SoftwareSerial rs485 (SSerialRX, SSerialTX);

/*-----( Declare Variables )-----*/
int byteReceived;
int byteSend;

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

const long refreshFrequency = 40000;
const long slaveWait = 10000;
const byte msgTrigger [] = "Start";
long lastReceived=-100000;
#endif





void readRS485() {
#if READ_RS485 == 1
  

  if (millis() - lastReceived > refreshFrequency) {
    digitalWrite (SSerialTxControl, HIGH);
    digitalWrite (SSerialTxControl2, HIGH);
    Serial.println("ask for slave RS485 update");
    myChannel.sendMsg (msgTrigger, sizeof (msgTrigger));
    rs485.flush();


   // delay(100);
    long startLoop = millis();
    digitalWrite (SSerialTxControl, LOW);
    digitalWrite (SSerialTxControl2, LOW);
    delay(100);
    Serial.println("wait answer from slave");
    while (millis() - startLoop < slaveWait) {

      // wait to be called
      if (myChannel.update())
      {

        digitalWrite (Pin13LED, HIGH);
        delay(50);
        digitalWrite (Pin13LED, LOW);
        delay(50);
        digitalWrite (Pin13LED, HIGH);
        delay(50);
        digitalWrite (Pin13LED, LOW);
        delay(50);
        Serial.println("Slave Message received");
        Serial.write (myChannel.getData(), myChannel.getLength());
        Serial.println();
        char msg[myChannel.getLength()];
        for (int i = 0; i < myChannel.getLength(); i++) {
          msg[i] = (char)myChannel.getData()[i];

        }
        msg[myChannel.getLength()] = '\0';

        Serial.println("RS485 send MQTT");
        MQTTsend("pool/ORP", msg);
        lastReceived = millis();

        break;
      }

    }


    Serial.println("timeout waiting answer");

  }
  //
  //  if (myChannel.update())
  //  {
  //    Serial.println("RS485 update");
  //    Serial.write (myChannel.getData(), myChannel.getLength());
  //    Serial.println();
  //
  //    char msg[myChannel.getLength()];
  //    for (int i = 0; i < myChannel.getLength(); i++) {
  //      msg[i] = (char)myChannel.getData()[i];
  //
  //    }
  //    //msg = myChannel.getData();
  //    msg[myChannel.getLength()] = '\0';
  //
  //    Serial.println("RS485 send MQTT");
  //    MQTTsend("pool/ORP", msg);
  //    //    digitalWrite (SSerialTxControl, HIGH);
  //    //    digitalWrite (SSerialTxControl2, HIGH);
  //    //    int retry = 1;
  //    //    while (retry > 0) {
  //    //      Serial.println("ack");
  //    //      myChannel.sendMsg (msgAck, sizeof (msgAck));
  //    //      delay(50);
  //    //      retry--;
  //    //    }
  //  }






#endif
}

