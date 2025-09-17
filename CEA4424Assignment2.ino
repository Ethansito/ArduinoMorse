#include "MorseEncoder.h"
#include "string.h"
#define INPUT_PIN 52
#define OUTPUT_PIN 53
MorseEncoder morseL = (OUTPUT_PIN); // Morse output pin
char needInput = 1; // !(If you need to print the "Enter Message" message)
String message; // The message to be transmitted
String receipt; // Message received
char prevInBit = 0;
long lastOn;
enum MORSE_STATES {WAIT, TRANSMIT, RECEIVE, HANDSHAKE_T, HANDSHAKE_R} MORSE_STATE;

void setup() {
  morseL.beginLight(5); // Sets speed of morse code in words per minute
  pinMode(INPUT_PIN, INPUT);
  Serial.begin(9600);
}

void onTick(){
  //Transition Switch
  long currentTime;
  char currentBit;
  switch (MORSE_STATE) {
    case WAIT: // Check for message to send or greeting from other Arduino
    currentBit = digitalRead(INPUT_PIN);
    currentTime = millis();
    if (currentBit){
      if (!prevInBit){lastOn = currentTime;}
      if (currentTime - lastOn > 2000){MORSE_STATE = HANDSHAKE_R;}
      }

    else if (message != ""){MORSE_STATE = HANDSHAKE_T;}
    prevInBit = currentBit;
    break;
    case HANDSHAKE_T: // Has greeting been received
    
    if (digitalRead(INPUT_PIN)){Serial.print("Handshake Returned. Beginning Transmit in 2s."); delay(2100); MORSE_STATE = TRANSMIT;}
    break;
    case HANDSHAKE_R: // Automatically transitions to RECEIVE
    break;
    case TRANSMIT: // Automatically Transitions to Wait after message sent
   
    break;
    case RECEIVE: // Automatically Transitions to Wait after message received
    break;
  }

  //Action Switch
  switch (MORSE_STATE) {
    case WAIT:
    if (needInput){Serial.print("\nEnter Message: ");}
    message = Serial.readString();
    if (message == ""){needInput = 0;} else{needInput = 1;}
    break;
    case HANDSHAKE_T:
    digitalWrite(OUTPUT_PIN, 1);
    delay(2100);
    digitalWrite(OUTPUT_PIN, 0);
    // The Arduino then waits to receive the greeting back before switching to Transmit state
    break;
    case HANDSHAKE_R:
    digitalWrite(OUTPUT_PIN, 1);
    delay(2100);
    digitalWrite(OUTPUT_PIN, 0);
    MORSE_STATE = RECEIVE;
    break;
    case TRANSMIT:
    Serial.print(MORSE_STATE);
    morseL.print(message);
    digitalWrite(OUTPUT_PIN, 1);
    delay(2100);
    digitalWrite(OUTPUT_PIN, 0);
    message = "";
    MORSE_STATE = WAIT;
    break;
    case RECEIVE:
    receipt = decode();
    Serial.println(receipt);
    MORSE_STATE = WAIT;
    break;
  }

}

String decode() {
  String receipt = "";
  char prevBit = 0;
  while (1){
    long currentTime = millis();
    long lastOnTime;
    long lastOffTime;
    char bit = digitalRead(INPUT_PIN);
    if (bit){ 
      if (!prevBit){
        lastOnTime = millis();
        long timeDiff = currentTime - lastOffTime; // How long the light was off
        if (timeDiff >= 1020){ // space = 1440 ms
          receipt += " ";
        } else if (timeDiff > 300 && timeDiff < 1020){
          receipt += "/";
        }
      }
    } else{ // If bit just turned off, identify info received
      if (prevBit){
        lastOffTime = millis();
        long timeDiff = currentTime - lastOnTime; // How long the light was on
        // dot = 240 ms dash = 960 space = 1440 ms letter space = 480 ms
        if (timeDiff < 300){ // dot
          receipt += ".";
        } else if (timeDiff > 540 && timeDiff < 1020){ // dash
          receipt += "-";
        } else if (timeDiff > 2000){ // Light on for 2 seconds = END MESSAGE
          MORSE_STATE = WAIT;
          if (receipt != "") {
            String msgOut = decodeMessage(receipt);
            Serial.println(msgOut);
            }
          return receipt;
        }
      }
    }
    prevBit = bit;
  }
}

void loop() {
  onTick();
}

const char* morseChart[] = {
  ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..",
  ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.",
  "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--.."
};

char getLetter(char code[]) {
  for(int i = 0; i < 26; i++) {
    if (strcmp(morseChart[i], code) == 0) {
      return 'A' + i;
    }
  }
  return '?';
}

String decodeMessage(String receipt) {
  String msgOut;
  char code[5] = "", msg[50] = "";
  int j = 0, k = 0;
  for (int i = 0; receipt[i] != '\0'; i++) {
    switch (receipt[i]) {
      case '.':
      case '-':
      code[j++] = receipt[i];
      break;

      case '/':
      code[j] = '\0';
      j = 0;
      msg[k++] = getLetter(code);
      break;

      case ' ':
      code[j] = '\0';
      j = 0;
      msg[k++] = getLetter(code);
      msg[k++] = ' ';
      break;
    }
  }
  if (j > 0) {
    code[j] = '\0';
    msg[k++] = getLetter(code);
    msg[k] = '\0';
  }
  msgOut = String(msg);
  return msgOut;
}
