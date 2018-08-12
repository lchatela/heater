#if MQTT_ON == 1
char MQTTserver[] = "192.168.0.77";
char result;
const unsigned long MQTTInterval = 60L * 1000L;
long MQTTlastAttempt = millis() + MQTTInterval;
long lastReconnectAttempt = 0;



void callback(char* topic, byte* payload, unsigned int length) {
  //  logMessage("Received a message", true);
  //  logMessage(topic, true);
  //  Serial.print("Message arrived [");
  //  Serial.print(topic);
  //
  //  Serial.print("] ");
  //  MQTTclient.publish("heater/message", strcat(prefix, topic));
  Serial.println(length);

  //  if (length == 1) {
  //    Serial.println(payload[0]);
  //    result = (char)payload[0];
  //    Serial.print(result);
  //  }
  char strResult[length + 1];
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    strResult[i] = (char)payload[i];

  }
  Serial.println();

  char prefix[17 + strlen(topic)];
  strcpy(prefix,"topic received ");
  strcat(prefix, topic);
  logMessage(prefix, true);
  char prefix2 [12 + length] = "message is  ";
  strResult[length] = '\0';
  logMessage(strResult, true);
  //MQTTclient.publish("heater/message", strcat(prefix2, strResult));
  updateRelay(topic, strResult );


}



boolean reconnect() {
  // Loop until we're reconnected
  int retry = 2;

  // do not retry if last connection test failed
  // to avoid delaying other processes
  if ((millis() - MQTTlastAttempt) < MQTTInterval) {
    return false;
  }

  while (!MQTTclient.connected()) {
    retry--;
    if (retry == 0) {
      MQTTlastAttempt = millis();
      break;
    }
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (MQTTclient.connect("arduinoClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      MQTTclient.publish("heater/message", "Arduino connected");
      // ... and resubscribe
      int returnValue;
      returnValue = MQTTclient.subscribe("pool/pump");
      Serial.println(returnValue);
      MQTTclient.subscribe("pool/projo");
      MQTTclient.subscribe("heater/temp/ecs");
      MQTTclient.subscribe("heater/temp/solarAccu");
    } else {
      Serial.print("failed, rc=");
      Serial.print(MQTTclient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  return MQTTclient.connected();
}

// check connection before sending
void MQTTsend(char * topic, char * message) {
  if (!MQTTclient.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
      } else {
        return;
      }
    }

  }

  if (MQTTclient.connected()) {
    MQTTclient.publish(topic, message);
  }

}


// check connection before sending
void MQTTsendFloat(char * topic, float val) {
  int i;

  int val_int;
  int val_fra;
  char buff[10];
#define DEC_PLACES 1000



  val_int = (int) val;   // compute the integer part of the float
  val_fra = (int) ((val - (float)val_int) * 1000);   // compute 3 decimal places (and convert it to int)
  snprintf (buff, sizeof(buff), "%d.%d", val_int, val_fra); //


  MQTTsend(topic, buff);


}

#endif // MQTT_ON




