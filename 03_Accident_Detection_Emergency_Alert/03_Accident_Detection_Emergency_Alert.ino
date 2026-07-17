#include <Wire.h>

/*
  Accident Detection and Emergency Alert - Arduino Uno R3

  Connections:
  MPU6050 SDA -> A4
  MPU6050 SCL -> A5
  Pulse sensor signal -> A0
  Buzzer -> D8
  Red LED -> D9
  Blue LED -> D10
  Green LED -> D11

  This is an educational prototype, not a medical or life-safety device.
  Calibrate IMPACT_THRESHOLD_G, TILT_THRESHOLD_DEG, and pulse thresholds.
*/

const byte MPU_ADDRESS = 0x68;
const byte PULSE_PIN = A0;
const byte BUZZER_PIN = 8;
const byte RED_LED_PIN = 9;
const byte BLUE_LED_PIN = 10;
const byte GREEN_LED_PIN = 11;

const float IMPACT_THRESHOLD_G = 2.5;
const float TILT_THRESHOLD_DEG = 60.0;
const int LOW_PULSE_THRESHOLD = 420;
const int HIGH_PULSE_THRESHOLD = 750;
const unsigned long CONFIRMATION_WINDOW_MS = 5000;

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

bool accidentDetected(float ax, float ay, float az) {
  const float magnitude = sqrt(ax * ax + ay * ay + az * az);
  const float tilt = atan2(sqrt(ax * ax + ay * ay), abs(az)) * 180.0 / PI;
  return magnitude >= IMPACT_THRESHOLD_G || tilt >= TILT_THRESHOLD_DEG;
}

bool pulseIsAbnormal() {
  const int pulseValue = analogRead(PULSE_PIN);
  Serial.print(F("Pulse sensor raw: "));
  Serial.println(pulseValue);
  return pulseValue < LOW_PULSE_THRESHOLD || pulseValue > HIGH_PULSE_THRESHOLD;
}

void setNormalStatus() {
  digitalWrite(GREEN_LED_PIN, HIGH);
  digitalWrite(BLUE_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  noTone(BUZZER_PIN);
}

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);

  Serial.begin(9600);
  Wire.begin();
  writeMpuRegister(0x6B, 0x00);
  setNormalStatus();
}

void loop() {
  float ax, ay, az;

  if (!readAcceleration(ax, ay, az)) {
    digitalWrite(BLUE_LED_PIN, HIGH);
    delay(250);
    return;
  }

  if (accidentDetected(ax, ay, az)) {
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(BLUE_LED_PIN, HIGH);

    const unsigned long startedAt = millis();
    bool abnormalPulseFound = false;

    while (millis() - startedAt < CONFIRMATION_WINDOW_MS) {
      if (pulseIsAbnormal()) {
        abnormalPulseFound = true;
        break;
      }
      delay(200);
    }

    if (abnormalPulseFound) {
      digitalWrite(BLUE_LED_PIN, LOW);
      digitalWrite(RED_LED_PIN, HIGH);
      tone(BUZZER_PIN, 2000);

      Serial.println(F("EMERGENCY: accident and abnormal pulse detected."));
      delay(5000);
    }
  }

  setNormalStatus();
  delay(100);
}
