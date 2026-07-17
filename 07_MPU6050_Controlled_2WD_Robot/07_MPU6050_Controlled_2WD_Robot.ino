#include <Wire.h>

/*
  MPU6050 Controlled 2WD Robot Car - Arduino Uno R3

  MPU6050:
  SDA -> A4
  SCL -> A5

  L298N:
  ENA -> D5
  IN1 -> D7
  IN2 -> D8
  ENB -> D6
  IN3 -> D9
  IN4 -> D10

  Motors must use their own battery supply. Join Arduino GND and L298N GND.
*/

const byte MPU_ADDRESS = 0x68;

const byte ENA_PIN = 5;
const byte ENB_PIN = 6;
const byte IN1_PIN = 7;
const byte IN2_PIN = 8;
const byte IN3_PIN = 9;
const byte IN4_PIN = 10;

const float DEAD_ZONE_DEG = 8.0;
const float MAX_TILT_DEG = 35.0;
const int MIN_SPEED = 90;
const int MAX_SPEED = 220;

float pitchOffset = 0;
float rollOffset = 0;

void writeMpuRegister(byte reg, byte value) {
  Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

bool readAngles(float &pitch, float &roll) {
  Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(0x3B);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(MPU_ADDRESS, (byte)6) != 6) return false;

  int16_t rawAx = (Wire.read() << 8) | Wire.read();
  int16_t rawAy = (Wire.read() << 8) | Wire.read();
  int16_t rawAz = (Wire.read() << 8) | Wire.read();

  float ax = rawAx / 16384.0;
  float ay = rawAy / 16384.0;
  float az = rawAz / 16384.0;

  pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / PI;
  roll = atan2(ay, az) * 180.0 / PI;
  return true;
}

void setMotor(byte inA, byte inB, byte enablePin, int speedValue) {
  speedValue = constrain(speedValue, -255, 255);

  if (speedValue > 0) {
    digitalWrite(inA, HIGH);
    digitalWrite(inB, LOW);
    analogWrite(enablePin, speedValue);
  } else if (speedValue < 0) {
    digitalWrite(inA, LOW);
    digitalWrite(inB, HIGH);
    analogWrite(enablePin, -speedValue);
  } else {
    digitalWrite(inA, LOW);
    digitalWrite(inB, LOW);
    analogWrite(enablePin, 0);
  }
}

void drive(int leftSpeed, int rightSpeed) {
  setMotor(IN1_PIN, IN2_PIN, ENA_PIN, leftSpeed);
  setMotor(IN3_PIN, IN4_PIN, ENB_PIN, rightSpeed);
}

void calibrate() {
  float pitchTotal = 0;
  float rollTotal = 0;
  int validSamples = 0;

  for (int i = 0; i < 100; i++) {
    float pitch, roll;
    if (readAngles(pitch, roll)) {
      pitchTotal += pitch;
      rollTotal += roll;
      validSamples++;
    }
    delay(10);
  }

  if (validSamples > 0) {
    pitchOffset = pitchTotal / validSamples;
    rollOffset = rollTotal / validSamples;
  }
}

int tiltToSpeed(float value) {
  float magnitude = abs(value);
  if (magnitude <= DEAD_ZONE_DEG) return 0;

  magnitude = constrain(magnitude, DEAD_ZONE_DEG, MAX_TILT_DEG);
  int speedValue = map(
      (long)(magnitude * 10),
      (long)(DEAD_ZONE_DEG * 10),
      (long)(MAX_TILT_DEG * 10),
      MIN_SPEED,
      MAX_SPEED
  );

  return value > 0 ? speedValue : -speedValue;
}

void setup() {
  pinMode(ENA_PIN, OUTPUT);
  pinMode(ENB_PIN, OUTPUT);
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(IN3_PIN, OUTPUT);
  pinMode(IN4_PIN, OUTPUT);

  Serial.begin(9600);
  Wire.begin();
  writeMpuRegister(0x6B, 0x00);

  drive(0, 0);
  delay(1000);
  calibrate();
}

void loop() {
  float pitch, roll;
  if (!readAngles(pitch, roll)) {
    drive(0, 0);
    return;
  }

  pitch -= pitchOffset;
  roll -= rollOffset;

  const int forwardSpeed = tiltToSpeed(pitch);
  const int steering = tiltToSpeed(roll) / 2;

  int leftSpeed = constrain(forwardSpeed + steering, -255, 255);
  int rightSpeed = constrain(forwardSpeed - steering, -255, 255);

  drive(leftSpeed, rightSpeed);

  Serial.print(F("Pitch: "));
  Serial.print(pitch);
  Serial.print(F(" | Roll: "));
  Serial.print(roll);
  Serial.print(F(" | Left: "));
  Serial.print(leftSpeed);
  Serial.print(F(" | Right: "));
  Serial.println(rightSpeed);

  delay(30);
}
