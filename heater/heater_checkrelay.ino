
#if CHECK_RELAY == 1
long lastRelayMillis = 0;
const unsigned long relayDelay = 20L * 1000L;
# endif //end check relay


void checkRelay() {
#if CHECK_RELAY == 1

  if (currentMillis - lastRelayMillis > relayDelay) {
    int pumpValue = ThingSpeak.readIntField(511114, 1, "DE8V7G7WCCDALMC9");
    int projectorValue = ThingSpeak.readIntField(511114, 2, "DE8V7G7WCCDALMC9");

    if (pumpValue == 1) {
      digitalWrite(relayPompePin, HIGH);
      logMessage("Pompe up", true);
      //Serial.println("pompe up");
    }
    else if (pumpValue == 0) {
      digitalWrite(relayPompePin, LOW);
      logMessage("Pompe down", true);
      //Serial.println("pompe down");

    }
    else {
      logMessage("Pompe unknown value - " + pumpValue, true);
      //Serial.print("pompe unknown value");
      //Serial.println(pumpValue);
    }

    if (projectorValue == 1) {
      digitalWrite(relayProjectorPin, LOW);
      logMessage("Projector up", true);
      //Serial.println("projector up");
    }
    else if (projectorValue == 0) {
      digitalWrite(relayProjectorPin, HIGH);
      logMessage("Projector down", true);
      //Serial.println("projector down");

    }
    else
    {
      logMessage("Projector unknown value -" + pumpValue, true);
      // Serial.print("pompe unknown value");
      // Serial.println(pumpValue);
    }
    lastRelayMillis = currentMillis;
  }

#endif //CHECK_RELAY
}

void updateRelay(char * topic, char * value) {

#if MQTT_ON == 1
  Serial.print("update Relay with [");
  Serial.print(topic);
  Serial.print("] = [");
  Serial.print(value);
  Serial.println("]");
  
  if (strcmp(topic,"pool/pump")==0) {
    Serial.println("entering pool/pump");
    if (strcmp(value,"1")==0) {
      digitalWrite(relayPompePin, HIGH);
    
      logMessage("Pompe up", true);
      
    } else if (strcmp(value,"0")==0) {
      digitalWrite(relayPompePin, LOW);
      
      logMessage("Pompe down", true);
      

    }
    else {
      Serial.print("error");
      Serial.println(value);
      logMessages("Pompe unknown value" , value, true);
      
    }
  }


  if (strcmp(topic ,"pool/projo") == 0){
    if (strcmp(value,"1")==0) {
      digitalWrite(relayProjectorPin, LOW);
      
      logMessage("Projo up", true);
      
    } else if (strcmp(value, "0")==0) {
      digitalWrite(relayProjectorPin, HIGH);
      logMessage("Projo down", true);
      

    }
    else {
      
      logMessages("projo unknown value " ,value, true);
    }
  }
#endif //MQTT_ON

}

