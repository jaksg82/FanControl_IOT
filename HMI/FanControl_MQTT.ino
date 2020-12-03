

void reconnect() {
  // Loop until we're reconnected
  while (!psClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (psClient.connect(mqttCLIENTID)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      psClient.publish("outTopic","hello world");
      // This is a workaround to address https://github.com/OPEnSLab-OSU/SSLClient/issues/9
      ethClientSSL.flush();
      // ... and resubscribe
      psClient.subscribe("inTopic");
      // This is a workaround to address https://github.com/OPEnSLab-OSU/SSLClient/issues/9
      ethClientSSL.flush();
    } else {
      Serial.print("failed, rc=");
      Serial.print(psClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
