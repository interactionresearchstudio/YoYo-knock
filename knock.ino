byte getKnockSequence() {

}

#define SOLENOID 13
#define TAPE_SIZE 200
byte knockArray[TAPE_SIZE / 8];
int tapeHeadPos = 0;

#define TAPE_SIZE_BYTES TAPE_SIZE/8


void recordBytes(int inputValue) {
  int tapeHeadPosBytes = tapeHeadPos / 8;
  bitWrite(knockArray[tapeHeadPosBytes], tapeHeadPos % 8, inputValue);
}

void playBytes(int inputValue) {
  int tapeHeadPosBytes = tapeHeadPos / 8;
  digitalWrite(SOLENOID, bitRead(knockArray[tapeHeadPosBytes], tapeHeadPos % 8));
}
