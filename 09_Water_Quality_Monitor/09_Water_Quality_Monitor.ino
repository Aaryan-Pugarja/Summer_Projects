/*
  Water Quality Monitor - Arduino Uno R3

  Connections:
  TDS sensor analog output -> A0
  Water-level sensor analog output -> A1
  Buzzer -> D8
  Red LED -> D9
  Blue LED -> D10
  Green LED -> D11

  Calibrate TDS_FACTOR and WATER_LOW_THRESHOLD using reference water samples.
  Do not immerse an exposed Arduino or breadboard.
*/

const byte TDS_PIN = A0;
const byte WATER_LEVEL_PIN = A1;
const byte BUZZER_PIN = 8;
const byte RED_LED_PIN = 9;
const byte BLUE_LED_PIN = 10;
const byte GREEN_LED_PIN = 11;

const float ADC_REFERENCE_V = 5.0;
const float WATER_TEMPERATURE_C = 25.0;
const float TDS_FACTOR = 0.5;

const int WATER_LOW_THRESHOLD = 250;
const float GOOD_TDS_MAX_PPM = 300.0;
const float ACCEPTABLE_TDS_MAX_PPM = 600.0;

float readTdsPpm() {
  long total = 0;
  const int sampleCount = 20;

  for (int i = 0; i < sampleCount; i++) {
    total += analogRead(TDS_PIN);
    delay(10);
  }

  const float averageAdc = total / (float)sampleCount;
  const float voltage = averageAdc * ADC_REFERENCE_V / 1023.0;

  const float compensationCoefficient =
      1.0 + 0.02 * (WATER_TEMPERATURE_C - 25.0);
  const float compensatedVoltage = voltage / compensationCoefficient;

  const float tds =
      (133.42 * compensatedVoltage * compensatedVoltage * compensatedVoltage
       - 255.86 * compensatedVoltage * compensatedVoltage
       + 857.39 * compensatedVoltage) * TDS_FACTOR;

  return max(0.0, tds);
}

void setIndicators(bool red, bool blue, bool green) {
  digitalWrite(RED_LED_PIN, red);
  digitalWrite(BLUE_LED_PIN, blue);
  digitalWrite(GREEN_LED_PIN, green);
}

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);

  Serial.begin(9600);
}

void loop() {
  const float tdsPpm = readTdsPpm();
  const int waterLevel = analogRead(WATER_LEVEL_PIN);
  const bool waterLow = waterLevel < WATER_LOW_THRESHOLD;

  if (waterLow || tdsPpm > ACCEPTABLE_TDS_MAX_PPM) {
    setIndicators(true, false, false);
    tone(BUZZER_PIN, 1800, 250);
  } else if (tdsPpm > GOOD_TDS_MAX_PPM) {
    setIndicators(false, true, false);
    noTone(BUZZER_PIN);
  } else {
    setIndicators(false, false, true);
    noTone(BUZZER_PIN);
  }

  Serial.print(F("TDS: "));
  Serial.print(tdsPpm, 0);
  Serial.print(F(" ppm | Water level raw: "));
  Serial.print(waterLevel);
  Serial.print(F(" | Low level: "));
  Serial.println(waterLow ? F("YES") : F("NO"));

  delay(1000);
}
