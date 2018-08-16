



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


  Serial.begin(115200);
  
#if ETHERNET_ON == 1
  Serial.println("Ethernet start");
  //logMessage("Ethernet start",true);
  ethernetConnection = true;
  if (Ethernet.begin(mac) == 0 ) {
    ethernetConnection = false;
    Serial.println("Connection failed");

  }
  ethernetConnectionTime = millis();

  delay(1000);

  Serial.println("connecting...");



  //Serial.println("ThingSpeak start");
  //if (ThingSpeak.begin(client)) {
  //  Serial.println("OK");
  //} else {

  //  Serial.println("nOK");
  //}


#endif




  //setup multiplexer
  DDRA = B11111111;
  ads.begin();
  ads.setGain(gain);

#if READ_RS485 == 1
  pinMode(SSerialTxControl, OUTPUT);
  pinMode(SSerialTxControl2, OUTPUT);
  rs485.begin(28800);


  //digitalWrite(SSerialTxControl, RS485Receive);  // Init Transceiver
  //digitalWrite(SSerialTxControl2, RS485Receive);
  // Start the software serial port, to another device
  myChannel.begin ();

  Serial.println("RS started");
#endif


#if MQTT_ON == 1
  MQTTclient.setServer(MQTTserver, 1883);
  MQTTclient.setCallback(callback);
#endif
  checkRelayAtRestart();
  logMessage("Arduino heater - restarted", true);
  

}


// __         ______    ______   _______
///  |       /      \  /      \ /       \ 
//$$ |      /$$$$$$  |/$$$$$$  |$$$$$$$  |
//$$ |      $$ |  $$ |$$ |  $$ |$$ |__$$ |
//$$ |      $$ |  $$ |$$ |  $$ |$$    $$/
//$$ |      $$ |  $$ |$$ |  $$ |$$$$$$$/
//$$ |_____ $$ \__$$ |$$ \__$$ |$$ |
//$$       |$$    $$/ $$    $$/ $$ |
//$$$$$$$$/  $$$$$$/   $$$$$$/  $$/




void loop() {

  if (ethernetConnection == false && millis() - ethernetConnectionTime > ethernetConnectionRetry) {
    Serial.println("Retrying connection");
    ethernetConnection = true;
    if (Ethernet.begin(mac) == 0 ) {
      ethernetConnection = false;
      Serial.println("Connection failed");

    } else {
      Serial.println("Connection established");
    }
    ethernetConnectionTime = millis();
  }

  ethernetMaintainValue = Ethernet.maintain();
  if (ethernetMaintainValue > 0) {
    Serial.print("Ethernet renewal, status:");
    Serial.println(ethernetMaintainValue);
  }

  if (firstLoop) {
    sendEmail("Arduino restarting", 0);
    firstLoop = false;
  }
  currentMillis = millis();

  readAnalog();
  checkAlertStatus();
  checkRelay();
  readRS485();
#if MQTT_ON == 1

  if (!MQTTclient.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    // Client connected

    MQTTclient.loop();
  }

#endif
  delay(10);


}


