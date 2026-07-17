/*
  Project 12: Automatic Door Lock
  Keypad-controlled door lock using a 4x4 keypad and SG90 servo.

  Components:
  - Arduino UNO
  - 4x4 Membrane Keypad
  - SG90 Servo Motor
  - 3V Buzzer

  Connections:
  Keypad rows R1-R4 -> Digital pins 2, 3, 4, 5
  Keypad columns C1-C4 -> Digital pins 6, 7, 8, 10

  Servo signal -> Digital pin 9
  Servo VCC    -> 5V
  Servo GND    -> GND

  Buzzer +     -> Digital pin 11
  Buzzer -     -> GND

  Controls:
  - Enter the password and press #
  - Press * to clear the entered password
  - Default password: 1234

  Required library:
  - Keypad by Mark Stanley and Alexander Brevig
*/

#include <Keypad.h>
#include <Servo.h>

const byte ROW_COUNT = 4;
const byte COLUMN_COUNT = 4;

char keys[ROW_COUNT][COLUMN_COUNT] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROW_COUNT] = {2, 3, 4, 5};
byte columnPins[COLUMN_COUNT] = {6, 7, 8, 10};

Keypad keypad = Keypad(
  makeKeymap(keys),
  rowPins,
  columnPins,
  ROW_COUNT,
  COLUMN_COUNT
);

Servo lockServo;

const byte SERVO_PIN = 9;
const byte BUZZER_PIN = 11;

const int LOCKED_ANGLE = 10;
const int UNLOCKED_ANGLE = 90;

const unsigned long UNLOCK_DURATION_MS = 5000;

const String CORRECT_PASSWORD = "1234";
String enteredPassword;

bool doorUnlocked = false;
unsigned long unlockedAt = 0;

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  lockServo.attach(SERVO_PIN);
  lockDoor();

  Serial.begin(9600);
  Serial.println("Automatic Door Lock");
  Serial.println("Enter password, then press #.");
}

void loop() {
  char key = keypad.getKey();

  if (key != NO_KEY) {
    handleKey(key);
  }

  if (doorUnlocked &&
      millis() - unlockedAt >= UNLOCK_DURATION_MS) {
    lockDoor();
    Serial.println("Door locked automatically.");
  }
}

void handleKey(char key) {
  if (key == '*') {
    enteredPassword = "";
    shortBeep();
    Serial.println("Password cleared.");
    return;
  }

  if (key == '#') {
    checkPassword();
    return;
  }

  if (key >= '0' && key <= '9') {
    if (enteredPassword.length() < 8) {
      enteredPassword += key;
      shortBeep();
      Serial.print('*');
    }
  }
}

void checkPassword() {
  Serial.println();

  if (enteredPassword == CORRECT_PASSWORD) {
    unlockDoor();
    successBeep();
    Serial.println("Correct password. Door unlocked.");
  } else {
    errorBeep();
    Serial.println("Incorrect password.");
  }

  enteredPassword = "";
}

void unlockDoor() {
  lockServo.write(UNLOCKED_ANGLE);
  doorUnlocked = true;
  unlockedAt = millis();
}

void lockDoor() {
  lockServo.write(LOCKED_ANGLE);
  doorUnlocked = false;
}

void shortBeep() {
  tone(BUZZER_PIN, 1200, 80);
}

void successBeep() {
  tone(BUZZER_PIN, 1800, 150);
  delay(200);
  tone(BUZZER_PIN, 2200, 150);
}

void errorBeep() {
  tone(BUZZER_PIN, 400, 500);
}
