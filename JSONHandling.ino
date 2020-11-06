#define MAX_NETWORKS_TO_SCAN 5

//JSON Data handling
bool inList;

void decodeData(const char* data) {
  // determines if data received from websocket is mac address or wifi credentials
  const size_t capacity = 2 * JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(2) + 150;
  DynamicJsonDocument doc(capacity);
  deserializeJson(doc, (const char*)data);
  if (doc.containsKey("mac")) {
    JsonArray mac = doc["mac"];
    String MAC = mac[0];
    Serial.println("I received a MAC address");
    Serial.println(MAC);
    if (MAC != "") {
      //save to preferences
      addToMacAddressJSON(MAC);
      if (currentSetupStatus != setup_client) {
        sendMacJSON();
      }
    } else {
      Serial.println("remote MAC address incorrect");
    }
  } else if (doc.containsKey("ssid")) {
    String remoteSSID = doc["ssid"][0];
    Serial.println("I received a SSID");
    if (remoteSSID != NULL) {
      if (doc.containsKey("password")) {
        String remotePASS = doc["password"][0];
        Serial.println("I received a Password");
        JsonArray wifi = doc["ssid"];
        for (int i = 0; i < wifi.size(); i++) {
          addToWiFiJSON(doc["ssid"][i], doc["password"][i]);
        }
      }
    } else {
      Serial.println("remote ssid empty");
    }
  } else {
    Serial.println("Incorrect data format");
    Serial.println(data);
  }
}

String getJSONMac() {
  //Gets mac address json from memory
  preferences.begin("scads", false);
  String requestBody = preferences.getString("mac", "");
  preferences.end();
  return (requestBody);
}

String getJSONWifi() {
  //Gets wifi json from memory
  preferences.begin("scads", false);
  String requestBody = preferences.getString("wifi", "");
  preferences.end();
  return (requestBody);
}

String setJSONMac(String macString) {
  //Sets mac address json from memory
  preferences.begin("scads", false);
  preferences.clear();
  preferences.putString("mac", macString);
  preferences.end();
}

void setJSONWifi(String wifiString ) {
  //Sets  wifi json from memory
  preferences.begin("scads", false);
  preferences.putString("wifi", wifiString);
  preferences.end();
}

void addToMacAddressJSON(String addr) {
  // appends mac address to memory json array if isn't already in it, creates the json array if it doesnt exist
  preferences.begin("scads", false);
  String macAddressList = preferences.getString("mac", "");
  if (macAddressList != "") {
    const size_t capacity = JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(1) + 10;
    DynamicJsonDocument addresses(capacity);
    deserializeJson(addresses, macAddressList);
    JsonArray mac = addresses["mac"];
    inList = false;
    for ( int i = 0; i < mac.size(); i++) {
      if (mac[i] == addr) {
        inList = true;
        Serial.println("mac address already in list");
        break;
      }
    }
    if (inList == false) {
      mac.add(addr);
      Serial.print("adding ");
      Serial.print(addr);
      Serial.println(" to the address list");
      macAddressList = "";
      serializeJson(addresses, macAddressList);
      Serial.println(macAddressList);
      preferences.putString("mac", macAddressList);
    }
  } else {
    const size_t capacity = JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(1) + 10;
    DynamicJsonDocument addresses(capacity);
    JsonArray macArray = addresses.createNestedArray("mac");
    macArray.add(addr);
    macAddressList = "";
    serializeJson(addresses, macAddressList);
    preferences.putString("mac", macAddressList);
    Serial.print("creating json object and adding the local mac ");
    Serial.print(addr);
    Serial.println(" to the address list");
  }
  preferences.end();
}

void addToWiFiJSON(String newSSID, String newPassword) {
  // appends wifi credentials to memory json array if isn't already in it, creates the array if it doesnt exist
  preferences.begin("scads", false);
  String wifilist = preferences.getString("wifi", "");
  if (wifilist != NULL) {
    const size_t capacity = 2 * JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(2) + 150;
    DynamicJsonDocument addresses(capacity);
    deserializeJson(addresses, wifilist);
    JsonArray ssids = addresses["ssid"];
    JsonArray passwords = addresses["password"];
    inList = false;
    for ( int i = 0; i < ssids.size(); i++) {
      if (ssids[i] == newSSID) {
        Serial.println("wifi credentials already in list");
        if (passwords[i] == newPassword) {
          inList = true;
          Serial.println("password same too");
          break;
        } else {
          inList = true;
          Serial.println("password will be updated");
          passwords[i] = newPassword;
          wifilist = "";
          serializeJson(addresses, wifilist);
          preferences.putString("wifi", wifilist);
          break;
        }
      }
    }
    if (inList == false) {
      ssids.add(newSSID);
      passwords.add(newPassword);
      Serial.print("adding ");
      Serial.print(newSSID);
      Serial.println(" to the wifi list");
      wifilist = "";
      serializeJson(addresses, wifilist);
      preferences.putString("wifi", wifilist);
    }
  } else {
    const size_t capacity = 2 * JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(2) + 150;
    DynamicJsonDocument addresses(capacity);
    JsonArray ssidArray = addresses.createNestedArray("ssid");
    ssidArray.add(newSSID);
    JsonArray passwordArray = addresses.createNestedArray("password");
    passwordArray.add(newPassword);
    wifilist = "";
    serializeJson(addresses, wifilist);
    preferences.putString("wifi", wifilist);
    Serial.print("creating json object and adding the local wificredentials");
  }
  preferences.end();
}

String getRemoteMacAddress(int clientID) {
  String remoteMacAddress = "";

  //Return a specific mac address from the JSON array
  String _macAddressJson = getJSONMac();
  const size_t capacity = JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(1) + 10;
  DynamicJsonDocument addresses(capacity);
  deserializeJson(addresses, _macAddressJson);
  JsonArray macs = addresses["mac"];

  if (macs.size() > clientID) {
    remoteMacAddress = macs[clientID].as<String>();
  }

  return (remoteMacAddress);
}

int getNumberOfMacAddresses() {
  int numberOfMacAddresses = 0;

  //Returns the number of mac address in JSON array
  preferences.begin("scads", false);
  String macCredentials = preferences.getString("mac", "");
  preferences.end();

  if (macCredentials != "") {
    const size_t capacity = JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(1) + 10;
    DynamicJsonDocument addresses(capacity);
    deserializeJson(addresses, macCredentials);
    numberOfMacAddresses = addresses["mac"].size();
  }

  return (numberOfMacAddresses);
}

String getScanAsJsonString() {
  String jsonString;

  StaticJsonDocument<1000> jsonDoc;
  getScanAsJson(jsonDoc);
  serializeJson(jsonDoc[0], jsonString);

  return (jsonString);
}

void getScanAsJson(JsonDocument& jsonDoc) {
  JsonArray networks = jsonDoc.createNestedArray();

  int n = WiFi.scanNetworks();
  n = (n > MAX_NETWORKS_TO_SCAN) ? MAX_NETWORKS_TO_SCAN : n;

  //Array is ordered by signal strength - strongest first
  for (int i = 0; i < n; ++i) {
    String networkSSID = WiFi.SSID(i);
    if (networkSSID.length() <= SSID_MAX_LENGTH) {
      JsonObject network  = networks.createNestedObject();
      network["SSID"] = WiFi.SSID(i);
      network["BSSID"] = WiFi.BSSIDstr(i);
      network["RSSI"] = WiFi.RSSI(i);
    }
  }
}
