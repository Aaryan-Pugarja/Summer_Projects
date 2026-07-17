#include <Wire.h>

/*
  Intelligent Liquid Volume Estimator - Arduino Uno R3

  Connections:
  HC-SR04 TRIG -> D8
  HC-SR04 ECHO -> D9
  MPU6050 SDA -> A4
  MPU6050 SCL -> A5

  Enter the inner dimensions of the cylindrical container below.
  Mount the ultrasonic sensor at the top, facing the liquid surface.
*/

const byte MPU_ADDRESS = 0x68;
const byte TRIG_PIN = 8;
const byte ECHO_PIN = 9;

const float CONTAINER_HEIGHT_CM = 30.0;
const float CONTAINER_RADIUS_CM = 10.0;
const float SENSOR_OFFSET_CM = 0.0;
const float MAX_RELIABLE_TILT_DEG = 12.0;

void writeMpuRegister(byte reg, byte value) {
  Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

bool readAcceleration(float &ax, float &ay, float &az) {
  Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(0x3B);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(MPU_ADDRESS, (byte)6) != 6) return false;

  int16_t rawAx = (Wire.read() << 8) | Wire.read();
  int16_t rawAy = (Wire.read() << 8) | Wire.read();
  int16_t rawAz = (Wire.read() << 8) | Wire.read();

  ax = rawAx / 16384.0;
  ay = rawAy / 16384.0;
  az = rawAz / 16384.0;
  return true;
}

float measureDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  const unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return -1.0;

  return duration * 0.0343 / 2.0;
}

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.begin(9600);
  Wire.begin();
  writeMpuRegister(0x6B, 0x00);
}

void loop() {
  float ax, ay, az;
  if (!readAcceleration(ax, ay, az)) {
    Serial.println(F("MPU6050 read failed."));
    delay(500);
    return;
  }

  const float tiltDeg =
      atan2(sqrt(ax * ax + ay * ay), abs(az)) * 180.0 / PI;

  const float measuredDistance = measureDistanceCm();
  if (measuredDistance < 0) {
    Serial.println(F("Ultrasonic measurement failed."));
    delay(500);
    return;
  }

  if (tiltDeg > MAX_RELIABLE_TILT_DEG) {
    Serial.print(F("Unreliable estimate: excessive tilt = "));
    Serial.print(tiltDeg);
    Serial.println(F(" degrees"));
    delay(500);
    return;
  }

  const float correctedDistance =
      (measuredDistance - SENSOR_OFFSET_CM) * cos(tiltDeg * PI / 180.0);

  const float liquidHeight =
      constrain(CONTAINER_HEIGHT_CM - correctedDistance, 0.0, CONTAINER_HEIGHT_CM);

  const float volumeCm3 =
      PI * CONTAINER_RADIUS_CM * CONTAINER_RADIUS_CM * liquidHeight;
  const float volumeLitres = volumeCm3 / 1000.0;

  Serial.print(F("Tilt: "));
  Serial.print(tiltDeg);
  Serial.print(F(" deg | Liquid height: "));
  Serial.print(liquidHeight);
  Serial.print(F(" cm | Volume: "));
  Serial.print(volumeLitres, 2);
  Serial.println(F(" L"));

  delay(500);
}
