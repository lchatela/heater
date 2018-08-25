#if CHECK_ALERT == 1
int buttonState = 0;         // variable for reading the pushbutton status
int nbPress = 0;
int lastPressCount = 0;
long previousMillis = 0;

long lastSentEmail = 0;

bool errorDetected = false;
bool emailSent = false;




char server[] = "mail.smtp2go.com";
int port = 2525;
const unsigned long emailInterval = 3600L * 1000L;
#endif

void checkAlertStatus() {
#if CHECK_ALERT == 1
  buttonState = digitalRead(buttonPin);


  // if nothing detected for one min
  if (currentMillis - previousMillis > 60000) {
    // if there was an error send stop email
    if (errorDetected == true) {
      logMessage("Error stopped -- sending email", true);
      //Serial.println(F("error stopped -- email"));
      errorDetected = false;
      errorType = 0;
      nbPress = 0;
      MQTTsendFloat("system/arduino1/ram",freeRam()); 
      sendEmail("Status ok", 0);
      MQTTsendFloat("system/arduino1/ram",freeRam()); 
      emailSent = false;
      digitalWrite(ledPin, LOW);
       resetFunc();
    }
   

  }




  // check if the pushbutton is pressed. If it is, the buttonState is LOW:
  if (buttonState == LOW) {

    errorDetected = true;
    nbPress = 1;
    
   // Serial.println("error detected");


    long startErrorCount = millis();
    //Serial.println(startErrorCount);
    logMessageAndInt("Error detected -start",startErrorCount, true);
    delay(800);
    //start looping to count nbPress
    for (int j = 0; j < 15; j++) {
      buttonState = digitalRead(buttonPin);
      if (buttonState == LOW) {
        nbPress++;
        //Serial.println("one more press");
        logMessage("one more press", true);
      }
      //break if we are monitoring for more than 2 secs
      //or max error number is 3
      if (millis() - startErrorCount > 3000 || nbPress == 3) {
        //Serial.println("breaking");
        logMessage("breaking", true);
        break;
      }
      delay(800);

    }


    errorType = nbPress;
    //Serial.print(F("Error Type "));
    //Serial.println(errorType);
    logMessageAndInt("Error type",errorType, true);
    nbPress = 0;
    if (!(emailSent) || (currentMillis - lastSentEmail > emailInterval)) {
      //Serial.println(F("Sending email"));
      logMessage("Sending error mail",true);
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

