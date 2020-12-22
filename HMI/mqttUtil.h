#ifndef MQTTUTIL_H_DEFINED
#define MQTTUTIL_H_DEFINED

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


bool reconnect(WiFiClient& wifi, PubSubClient& psClient, const char* clientID, const char* user, const char* pass, const char* inTopic) {
  // Loop until we're reconnected
    Debug::print("Attempting MQTT connection...");
    Debug::println(clientID);
    // Attempt to connect
    if (psClient.connect(clientID, user, pass)) {
      Debug::println("connected");
      // Once connected, publish an announcement...
      //psClient.publish(mqttTopicRoot,"hello world");
      // This is a workaround to address https://github.com/OPEnSLab-OSU/SSLClient/issues/9
      wifi.flush();
      // ... and resubscribe
      psClient.subscribe(inTopic);
      // This is a workaround to address https://github.com/OPEnSLab-OSU/SSLClient/issues/9
      wifi.flush();
    } else {
      Debug::print("failed, rc=");
      Debug::print(psClient.state());
      Debug::println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      //delay(5000);
      return psClient.connected();
    }
}

bool connectWiFi(const char* ssid, const char* pass) {
  Debug::print("Attempting to connect to SSID: ");
  Debug::print(ssid);
  Debug::print(" ");

  if (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Debug::println("failed");
    return false;
  }
  Debug::println("You're connected to the network");
  return true;
}
#endif
