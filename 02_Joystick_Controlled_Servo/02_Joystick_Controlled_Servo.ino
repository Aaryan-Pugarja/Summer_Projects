#include <Servo.h>

/*
  Joystick Controlled Servo - Arduino Uno R3

  Connections:
  Joystick VRx -> A0
  Joystick SW  -> D2
  Servo signal -> D9
  Joystick and servo grounds must be common.

  Hold the joystick button during startup to calibrate its center.
  Prefer a separate regulated 5V supply for the servo.
*/

const byte JOYSTICK_X_PIN = A0;
const byte JOYSTICK_SW_PIN = 2;
const byte SERVO_PIN = 9;

const int DEAD_ZONE = 35;
const int SERVO_MIN_ANGLE = 10;
const int SERVO_MAX_ANGLE = 170;

Servo steeringServo;
int joystickCenter = 512;

void calibrateJoystick() {
  long total = 0;
  const int samples = 100;

  for (int i = 0; i < samples; i++) {
    total += analogRead(JOYSTICK_X_PIN);
    delay(5);
  }

  joystickCenter = total / samples;
}

void setup() {
  pinMode(JOYSTICK_SW_PIN, INPUT_PULLUP);
  Serial.begin(9600);

  steeringServo.attach(SERVO_PIN);
  steeringServo.write(90);

  if (digitalRead(JOYSTICK_SW_PIN) == LOW) {
    calibrateJoystick();
  }
}

void loop() {
  int raw = analogRead(JOYSTICK_X_PIN);
  int corrected = raw - joystickCenter;

  if (abs(corrected) <= DEAD_ZONE) {
    corrected = 0;
  }

  int angle;
  if (corrected < 0) {
    angle = map(corrected, -joystickCenter, 0, SERVO_MIN_ANGLE, 90);
  } else {
    angle = map(corrected, 0, 1023 - joystickCenter, 90, SERVO_MAX_ANGLE);
  }

  angle = constrain(angle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
  steeringServo.write(angle);

  Serial.print(F("Joystick: "));
  Serial.print(raw);
  Serial.print(F(" | Center: "));
  Serial.print(joystickCenter);
  Serial.print(F(" | Servo: "));
  Serial.println(angle);

  delay(20);
}
