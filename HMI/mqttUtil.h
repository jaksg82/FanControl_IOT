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


bool reconnect() {
  // Loop until we're reconnected
    Debug::print("Attempting MQTT connection...");
    Debug::println(mqttCLIENTID);
    // Attempt to connect
    if (psClient.connect(mqttCLIENTID, mqttUSERNAME, mqttPASSWORD)) {
      Debug::println("connected");
      // Once connected, publish an announcement...
      psClient.publish(mqttTopicRoot,"hello world");
      // This is a workaround to address https://github.com/OPEnSLab-OSU/SSLClient/issues/9
      wifiClient.flush();
      // ... and resubscribe
      psClient.subscribe(mqttTopicRoot);
      // This is a workaround to address https://github.com/OPEnSLab-OSU/SSLClient/issues/9
      wifiClient.flush();
    } else {
      Debug::print("failed, rc=");
      Debug::print(psClient.state());
      Debug::println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      //delay(5000);
      return psClient.connected();
    }
}

void connectWiFi() {
  Debug::print("Attempting to connect to SSID: ");
  Debug::print(networkSSID);
  Debug::print(" ");

  while (WiFi.begin(networkSSID, networkPASSWORD) != WL_CONNECTED) {
    // failed, retry
    Debug::print(".");
    delay(5000);
  }
  Debug::println();

  Debug::println("You're connected to the network");
  Debug::println();
}
