/*  Callback function to handle the incoming messages from the MQTT broker */
void callback(char* topic, byte* payload, unsigned int length) {
  Debug::print("Message arrived [");
  Debug::print(topic);
  Debug::print("] ");
  for (int i=0;i<length;i++) {
    Debug::print((char)payload[i]);
  }
  Debug::println();
}


void reconnect() {
  // Loop until we're reconnected
    Debug::print("Attempting MQTT connection...");
    // Attempt to connect
    if (psClient.connect(mqttCLIENTID)) {
      Debug::println("connected");
      // Once connected, publish an announcement...
      psClient.publish("outTopic","hello world");
      // This is a workaround to address https://github.com/OPEnSLab-OSU/SSLClient/issues/9
      ethClientSSL.flush();
      // ... and resubscribe
      psClient.subscribe("inTopic");
      // This is a workaround to address https://github.com/OPEnSLab-OSU/SSLClient/issues/9
      ethClientSSL.flush();
    } else {
      Debug::print("failed, rc=");
      Debug::print(psClient.state());
      Debug::println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
}

void connectWiFi() {
  Debug::print("Attempting to connect to SSID: ");
  Debug::print(SECRET_SSID);
  Debug::print(" ");

  while (WiFi.begin(SECRET_SSID, SECRET_PASS) != WL_CONNECTED) {
    // failed, retry
    Debug::print(".");
    delay(5000);
  }
  Debug::println();

  Debug::println("You're connected to the network");
  Debug::println();
}