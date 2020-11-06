byte getKnockSequence() {

}

#define SOLENOID 13
#define TAPE_SIZE 200
byte knockArray[TAPE_SIZE / 8] = {101, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int tapeHeadPos = 0;

#define TAPE_SIZE_BYTES TAPE_SIZE/8

int updateIntervalMs = 20;
unsigned long updateAtMs = 0;
#define NOPIEZO
#define PIEZO_TEST_BUTTON 32
#define PIEZO 4
typedef enum {
  RECORD,
  PLAY,
  INACTIVE
} State;

State currentState = INACTIVE;
void playKnock(){
  for(int i = 0; i < TAPE_SIZE;i++){
    playBytes(i);
    delay(updateIntervalMs);
  }
  digitalWrite(SOLENOID,0);
  
}
void knockCheck() {
  if (millis() - updateAtMs > updateIntervalMs ) {
    updateAtMs = millis();

    //make input binary
#ifndef NOPIEZO
    int inputValue = analogRead(PIEZO);
    if (inputValue > 300) {
      inputValue = 1;
    } else {
      inputValue = 0;
    }
#else
    int inputValue = digitalRead(PIEZO_TEST_BUTTON);
    if (inputValue) {
      inputValue = 0;
    } else {
      inputValue = 1;
    }
#endif

    updateState(inputValue);


    switch (currentState) {
      case RECORD:
        recordBytes(inputValue);
        playBytes(tapeHeadPos);
        tapeHeadPos++;
        if (tapeHeadPos == TAPE_SIZE) {
          currentState = INACTIVE;
          tapeHeadPos = 0;
          Serial.println("Send knock sequence!");
          socketIO_sendKnocks();
        }
        break;
      case PLAY:
        break;
      case INACTIVE:
        break;
    }
  }
}

void updateState(int inputValue) {
  if (inputValue == 1 && currentState == INACTIVE) {
    currentState = RECORD;
    Serial.println("Start recording");
  }
}

void recordBytes(int inputValue) {
  int tapeHeadPosBytes = tapeHeadPos / 8;
  bitWrite(knockArray[tapeHeadPosBytes], tapeHeadPos % 8, inputValue);
}

void playBytes(int tapeHeadPos) {
  int tapeHeadPosBytes = tapeHeadPos / 8;
  digitalWrite(SOLENOID, bitRead(knockArray[tapeHeadPosBytes], tapeHeadPos % 8));
}
