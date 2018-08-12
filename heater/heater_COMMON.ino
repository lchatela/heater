void logMessageAndInt(const char* message, int value, bool ln) {
  char msg[strlen(message)+16];
  strcat(msg,message);
  char intStr[16];
  
  itoa(value,intStr,10);
  strcat(msg,intStr);
  
  logMessage(msg, ln);
}


void logMessage(const char* message, bool ln) {
  if (ln) {
    Serial.println(message);
  } else {
    Serial.print(message);
  }
#if MQTT_ON == 1
  

  if (!MQTTclient.connected()) {
    reconnect();
  }
  MQTTclient.publish("heater/message", message);
#endif
}

void logMessages( char* message, char * details, bool ln) {
  int lg=strlen(message)+ strlen(details);
  char fullMessage[lg];
  //fullMessage= message;
  strcpy(fullMessage, message);
  strcat(fullMessage,details);
  
  if (ln) {
    Serial.println(fullMessage);
  } else {
    Serial.print(fullMessage);
  }
#if MQTT_ON == 1
  
  MQTTsend("heater/message", fullMessage);
  
#endif
}

