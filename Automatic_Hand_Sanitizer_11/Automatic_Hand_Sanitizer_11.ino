/*
  Project 11: Automatic Hand Sanitizer

  Components:
  - Arduino UNO
  - IR Proximity Sensor
  - SG90 Servo Motor

  Connections:
  IR sensor VCC    -> 5V
  IR sensor GND    -> GND
  IR sensor OUT    -> Digital pin 2

  Servo red wire   -> 5V
  Servo brown/black wire -> GND
  Servo orange/yellow wire -> Digital pin 9

  Note:
  Many IR proximity modules output LOW when an object is detected.
  Change OBJECT_DETECTED_STATE to HIGH if your module works oppositely.
*/

#include <Servo.h>

const byte IR_SENSOR_PIN = 2;
const byte SERVO_PIN = 9;

const byte OBJECT_DETECTED_STATE = LOW;

const int REST_ANGLE = 10;
const int DISPENSE_ANGLE = 90;

const unsigned long DISPENSE_HOLD_MS = 800;
const unsigned long COOLDOWN_MS = 1500;

Servo dispenserServo;

bool waitingForHandRemoval = false;
unsigned long lastDispenseAt = 0;

void setup() {
  pinMode(IR_SENSOR_PIN, INPUT);

  dispenserServo.attach(SERVO_PIN);
  dispenserServo.write(REST_ANGLE);

  Serial.begin(9600);
  Serial.println("Automatic Hand Sanitizer");
}

void loop() {
  bool handDetected =
      digitalRead(IR_SENSOR_PIN) == OBJECT_DETECTED_STATE;

  bool cooldownFinished =
      millis() - lastDispenseAt >= COOLDOWN_MS;

  if (handDetected && !waitingForHandRemoval && cooldownFinished) {
    dispenseSanitizer();
    waitingForHandRemoval = true;
    lastDispenseAt = millis();
  }

  if (!handDetected) {
    waitingForHandRemoval = false;
  }

  delay(30);
}

void dispenseSanitizer() {
  Serial.println("Hand detected. Dispensing sanitizer.");

  dispenserServo.write(DISPENSE_ANGLE);
  delay(DISPENSE_HOLD_MS);

  dispenserServo.write(REST_ANGLE);
}
