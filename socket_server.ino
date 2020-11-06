//Local socket server
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {

  if (type == WS_EVT_CONNECT) {
    currentPairedStatus = localSetup;
    Serial.println("Websocket client connection received");
    webSocketClientID = client->id();
    Serial.println(client->id());
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.println("Client disconnected");
    if (getNumberOfMacAddresses() >= 2) {
      currentPairedStatus = pairedSetup;
    }
  } else if (type == WS_EVT_DATA) {
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len) {
      //the whole message is in a single frame and we got all of it's data
      Serial.println(client->id());
      if (info->opcode == WS_TEXT) {
        data[len] = 0;
        Serial.println((char*)data);
        decodeData((char*)data);
      }
    }
  }
}

void sendMacJSON() {
  socket_server.textAll(getJSONMac());
  Serial.println("Sending mac addresses");
}

void sendWifiCredentials() {
  //socket_server.text(webSocketClientID, (char*)text);
  // okay while we only have 1 client
  socket_server.textAll(getJSONWifi());
}

void setupLocalServer() {
  socket_server.onEvent(onWsEvent);
  server.addHandler(&socket_server);
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);//only when requested from AP
  server.begin();
  Serial.println("Local Socket server started");
}
