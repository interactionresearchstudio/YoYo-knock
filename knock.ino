void playKnock() {
  currentState = PLAY;
}
void knockCheck() {
  if (millis() - updateAtMs > updateIntervalMs ) {
    updateAtMs = millis();

    //make input binary
#ifndef NOPIEZO
    int inputValue = analogRead(PIEZO);
    if (inputValue > 200) {
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
        playBytes(tapeHeadPos);
        tapeHeadPos++;
        if (tapeHeadPos == TAPE_SIZE) {
          currentState = INACTIVE;
          tapeHeadPos = 0;
          digitalWrite(SOLENOID, 0);
        }
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
