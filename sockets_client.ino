//Client local socket

void setupSocketClientEvents() {
  socket_client.begin("192.168.4.1", 80, "/ws");
  socket_client.onEvent(webSocketEvent);
  socket_client.setReconnectInterval(5000);
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

  switch (type) {
    case WStype_DISCONNECTED:
      Serial.println("[WSc] Disconnected!\n");
      //Hot fix for when client doesn't catch RESTART command
      softReset(4000);
      break;
    case WStype_CONNECTED:
      Serial.println("Connected!");
      socket_client.sendTXT(getJSONMac().c_str());
      break;
    case WStype_TEXT:
      Serial.println("Text received");
      #ifdef DEV
      Serial.println((char *)payload);
      #endif
      String output = (char *)payload;
      if (output == "RESTART") {
        softReset(4000);
        Serial.println("i'm going to reset");
      } else {
        decodeData((char *)payload);
      }
      break;
  }
}
