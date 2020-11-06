
void socketIO_Connected(const char * payload, size_t length) {
  Serial.println("Socket.IO Connected!");
  blinkOnConnect();
}

void socketIO_sendMac(const char * payload, size_t length) {
  Serial.println("GOT MAC REQUEST");
  const size_t capacity = JSON_OBJECT_SIZE(1) + 50;
  DynamicJsonDocument doc(capacity);
  doc["macAddress"] =  myID;
  String bodyReq;
  serializeJson(doc, bodyReq);
  Serial.println(bodyReq);
  socketIO.emit("mac", bodyReq.c_str());
}

void socketIO_event(const char * payload, size_t length) {
  Serial.print("got message: ");
  Serial.println(payload);
}

void socketIO_msg(const char * payload, size_t length) {
  Serial.println("got msg");
  const size_t capacity = JSON_ARRAY_SIZE(25) + 2*JSON_OBJECT_SIZE(2) + 60;
  DynamicJsonDocument incomingDoc(capacity);
  deserializeJson(incomingDoc, payload);
  const char* recMacAddress = incomingDoc["macAddress"];
  const char* data_project = incomingDoc["data"]["project"];

  Serial.print("I got a message from ");
  Serial.println(recMacAddress);
  Serial.print("Which is of type ");
  Serial.println(data_project);

  if (String(data_project) == "test") {
    blinkDevice();
  } else if (String(data_project) == "knockknock") {
    for (int i = 0; i < TAPE_SIZE / 8; i++) {
      knockArray[i] = incomingDoc["data"]["knock"][i];
    }
    playKnock();
  }
}

void socketIO_sendButtonPress() {
  Serial.println("button send");
  const size_t capacity = JSON_ARRAY_SIZE(25) + 2*JSON_OBJECT_SIZE(2) + 60;
  DynamicJsonDocument doc(capacity);
  doc["macAddress"] = getRemoteMacAddress(1);
  JsonObject data = doc.createNestedObject("data");
  data["project"] = "test";
  String sender;
  serializeJson(doc, sender);
  socketIO.emit("msg", sender.c_str());
}

void socketIO_sendKnocks() {
  Serial.println("Knock Sequence send");
  const size_t capacity = JSON_ARRAY_SIZE(25) + 2*JSON_OBJECT_SIZE(2) + 60;
  DynamicJsonDocument doc(capacity);
  doc["macAddress"] = getRemoteMacAddress(1);
  JsonObject data = doc.createNestedObject("data");
  data["project"] = "knockknock";
  for (int i = 0; i < TAPE_SIZE / 8; i ++) {
    data["knock"].add(knockArray[i]);
  }
  String sender;
  serializeJson(doc, sender);
  socketIO.emit("msg", sender.c_str());
}

void setupSocketIOEvents() {
  // Setup 'on' listen events
  socketIO.on("connect", socketIO_Connected);
  socketIO.on("event", socketIO_event);
  socketIO.on("send mac", socketIO_sendMac);
  socketIO.on("msg", socketIO_msg);
  socketIO.begin(host, port, path);
  Serial.println("attached socketio listeners");
}
