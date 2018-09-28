
#if CHECK_ALERT
byte sendEmail(String subject, int errType)
{
  byte thisByte = 0;
  byte respCode;

  if (client.connect(server, port) == 1) {
    Serial.println(F("connected"));
  } else {
    logMessage("Email connection failed", true);
    //Serial.println(F("connection failed"));
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
  client.println(F("removed=="));
  //
  if (!eRcv()) return 0;
  //
  Serial.println(F("Sending Password"));
  //// change to your base64 encoded password
  client.println(F("removed"));
  //
  if (!eRcv()) return 0;
  //
  //// change to your email address (sender)
  Serial.println(F("Sending From"));
  client.println(F("MAIL From: <error@oko.com>"));
  if (!eRcv()) return 0;
  //
  //// change to recipient address
  Serial.println(F("Sending To"));
  client.println(F("RCPT To: <toto@gmail.com>"));
  if (!eRcv()) return 0;
  //
  Serial.println(F("Sending DATA"));
  client.println(F("DATA"));
  if (!eRcv()) return 0;
  //
  Serial.println(F("Sending email"));
  //
  // change to recipient address
  client.println(F("To: You <toto@gmail.com>"));
  //
  // change to your address
  client.println(F("From: Chaudiere <error@oko.com>"));
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
      logMessage("Timeout in eRcv",true);
      //Serial.println(F("\r\nTimeout"));
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
      logMessage("Timeout in eFail",true);
      //Serial.println(F("\r\nTimeout"));
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

