//#define DEV
//#define STAGING

#define FADE_3 22
#define FADE_1 21

#define EXTERNAL_BUTTON 23
#define CAPTOUCH T0

#define LED_BUILTIN 2
#define LED_BUILTIN_ON HIGH

int BUTTON_BUILTIN =  0;

bool disconnected = false;

unsigned long wificheckMillis;
unsigned long wifiCheckTime = 5000;


enum PAIRED_STATUS {
  remoteSetup,
  localSetup,
  pairedSetup
};
int currentPairedStatus = remoteSetup;

enum SETUP_STATUS {
  setup_pending,
  setup_client,
  setup_server,
  setup_finished
};
int currentSetupStatus = setup_pending;

#define PROJECT_SLUG "ESP32-SOCKETIO"
#define VERSION "v0.2"
#define ESP32set
#define WIFICONNECTTIMEOUT 60000
#define SSID_MAX_LENGTH 31

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

//socketIO
#include <SocketIoClient.h>
/* HAS TO BE INITIALISED BEFORE WEBSOCKETSCLIENT LIB */
SocketIoClient socketIO;

//Local Websockets
#include <WebSocketsClient.h>

#include <Preferences.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiAP.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>

#include <AceButton.h>
using namespace ace_button;

#include "SPIFFS.h"

//Access Point credentials
String scads_ssid = "";
String scads_pass = "blinkblink";

const byte DNS_PORT = 53;
DNSServer dnsServer;
IPAddress apIP(192, 168, 4, 1);

Preferences preferences;

//local server
AsyncWebServer server(80);
AsyncWebSocket socket_server("/ws");

//local websockets client
WebSocketsClient socket_client;
byte webSocketClientID;

WiFiMulti wifiMulti;

String wifiCredentials = "";
String macCredentials = "";

/// Led Settings ///
bool isBlinking = false;
bool readyToBlink = false;
unsigned long blinkTime;
int blinkDuration = 200;

// Touch settings and config
class CapacitiveConfig: public ButtonConfig {
  public:
    uint8_t _pin;
    uint16_t _threshold;
    CapacitiveConfig(uint8_t pin, uint16_t threshold) {
      _pin = pin;
      _threshold = threshold;
    }
    void setThreshold(uint16_t CapThreshold) {
      _threshold = CapThreshold;
    }
  protected:
    int readButton(uint8_t /*pin*/) override {
      uint16_t val = touchRead(_pin);
      return (val < _threshold) ? LOW : HIGH;
    }

};

int TOUCH_THRESHOLD = 60;
int TOUCH_HYSTERESIS = 20;
#define LONG_TOUCH 1500
CapacitiveConfig touchConfig(CAPTOUCH, TOUCH_THRESHOLD);
AceButton buttonTouch(&touchConfig);
bool isSelectingColour = false;

//Button Settings
AceButton buttonBuiltIn(BUTTON_BUILTIN);
AceButton buttonExternal(EXTERNAL_BUTTON);

void handleButtonEvent(AceButton*, uint8_t, uint8_t);
int longButtonPressDelay = 5000;

//reset timers
bool isResetting = false;
unsigned long resetTime;
int resetDurationMs = 4000;

String myID = "";

/// Socket.IO Settings ///
#ifndef STAGING
char host[] = "irs-socket-server.herokuapp.com"; // Socket.IO Server Address
#else
char host[] = "irs-socket-server-staging.herokuapp.com"; // Socket.IO Staging Server Address
#endif
int port = 80; // Socket.IO Port Address
char path[] = "/socket.io/?transport=websocket"; // Socket.IO Base Path

void setup() {
  Serial.begin(115200);
  setupPins();
  setupCapacitiveTouch();

  //create 10 digit ID
  myID = generateID();

  SPIFFS.begin();

  preferences.begin("scads", false);
  wifiCredentials = preferences.getString("wifi", "");
  macCredentials = preferences.getString("mac", "");
  preferences.end();

  setPairedStatus();

  if (wifiCredentials == "" || getNumberOfMacAddresses() < 2) {
    Serial.println("Scanning for available SCADS");
    boolean foundLocalSCADS = scanAndConnectToLocalSCADS();
    if (!foundLocalSCADS) {
      //become server
      currentSetupStatus = setup_server;
      createSCADSAP();
      setupCaptivePortal();
      setupLocalServer();
    }
    else {
      //become client
      currentSetupStatus = setup_client;
      setupSocketClientEvents();
    }
  }
  else {
    Serial.print("List of Mac addresses:");
    Serial.println(macCredentials);
    //connect to router to talk to server
    digitalWrite(LED_BUILTIN, 0);
    connectToWifi(wifiCredentials);
    //checkForUpdate();
    setupSocketIOEvents();
    currentSetupStatus = setup_finished;
    Serial.println("setup complete");
  }
}

void setPairedStatus() {
  int numberOfMacAddresses = getNumberOfMacAddresses();
  if (numberOfMacAddresses == 0) {
    Serial.println("setting up JSON database for mac addresses");
    preferences.clear();
    addToMacAddressJSON(myID);
  }
  else if (numberOfMacAddresses < 2) {
    //check it has a paired mac address
    Serial.println("Already have local mac address in preferences, but nothing else");
  }
  else {
    currentPairedStatus = pairedSetup;
    Serial.println("Already has one or more paired mac address");
  }
}

String getCurrentPairedStatusAsString() {
  String currentPairedStatusAsString = "";

  switch (currentPairedStatus) {
    case remoteSetup:      currentPairedStatusAsString = "remoteSetup"; break;
    case localSetup:       currentPairedStatusAsString = "localSetup";  break;
    case pairedSetup:        currentPairedStatusAsString = "pairedSetup";   break;
  }

  return (currentPairedStatusAsString);
}

void loop() {
  switch (currentSetupStatus) {
    case setup_pending:
      break;
    case setup_client:
      socket_client.loop();
      break;
    case setup_server:
      socket_server.cleanupClients();
      dnsServer.processNextRequest();
      break;
    case setup_finished:
      socketIO.loop();
      ledHandler();
      wifiCheck();
      knockCheck();
      break;
  }
  buttonBuiltIn.check();
  buttonExternal.check();
  buttonTouch.check();
  checkReset();
}
