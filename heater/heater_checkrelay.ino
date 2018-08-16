
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

  if (strcmp(topic, "pool/pump") == 0) {
    Serial.println("entering pool/pump");
    if (strcmp(value, "1") == 0) {
      digitalWrite(relayPompePin, HIGH);

      logMessage("Pompe up", true);
      EEPROM.write(0, 1);

    } else if (strcmp(value, "0") == 0) {
      digitalWrite(relayPompePin, LOW);

      logMessage("Pompe down", true);
      EEPROM.write(0, 0);


    }
    else {
      Serial.print("error");
      Serial.println(value);
      logMessages("Pompe unknown value" , value, true);

    }
  }


  if (strcmp(topic , "pool/projo") == 0) {
    if (strcmp(value, "1") == 0) {
      digitalWrite(relayProjectorPin, LOW);

      logMessage("Projo up", true);
      EEPROM.write(1, 1);

    } else if (strcmp(value, "0") == 0) {
      digitalWrite(relayProjectorPin, HIGH);
      logMessage("Projo down", true);
      EEPROM.write(1, 0);


    }
    else {

      logMessages("projo unknown value " , value, true);
    }
  }
#endif //MQTT_ON



}

void checkRelayAtRestart() {
  //get EEPROM values
  int value = 0;

  //0 is for the pump
  value = EEPROM.read(0);
  Serial.print("pump is stored as ");
  Serial.println(value);
  if (value == 1) {
    digitalWrite(relayPompePin, HIGH);
  } else {
    digitalWrite(relayPompePin, LOW);
  }

  //1 is for the projo
  value = EEPROM.read(1);
  Serial.print("projo is stored as ");
  Serial.println(value);
  if (value == 1) {
    digitalWrite(relayProjectorPin, LOW);
  } else {
    digitalWrite(relayProjectorPin, HIGH);
  }
}

