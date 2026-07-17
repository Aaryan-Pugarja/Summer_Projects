#include <DHT.h>

/*
  Industrial Gas and Temperature Monitor - Arduino Uno R3

  Install: "DHT sensor library" by Adafruit.

  Connections:
  MQ-2 analog output -> A0
  DHT11 data -> D2
  Buzzer -> D8
  Red LED -> D9
  Blue LED -> D10
  Green LED -> D11

  Allow the MQ-2 to warm up and calibrate GAS_WARNING_LEVEL.
*/

#define DHT_TYPE DHT11

const byte MQ2_PIN = A0;
const byte DHT_PIN = 2;
const byte BUZZER_PIN = 8;
const byte RED_LED_PIN = 9;
const byte BLUE_LED_PIN = 10;
const byte GREEN_LED_PIN = 11;

const int GAS_WARNING_LEVEL = 350;
const int GAS_DANGER_LEVEL = 550;
const float TEMPERATURE_WARNING_C = 35.0;
const float TEMPERATURE_DANGER_C = 45.0;

DHT dht(DHT_PIN, DHT_TYPE);

void showNormal() {
  digitalWrite(GREEN_LED_PIN, HIGH);
  digitalWrite(BLUE_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  noTone(BUZZER_PIN);
}

void showWarning() {
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(BLUE_LED_PIN, HIGH);
  digitalWrite(RED_LED_PIN, LOW);
  tone(BUZZER_PIN, 1200, 150);
}

void showDanger() {
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(BLUE_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, HIGH);
  tone(BUZZER_PIN, 2200);
}

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);

  Serial.begin(9600);
  dht.begin();
}

void loop() {
  const int gasValue = analogRead(MQ2_PIN);
  const float temperature = dht.readTemperature();
  const float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println(F("DHT11 read failed."));
    delay(2000);
    return;
  }

  const bool danger = gasValue >= GAS_DANGER_LEVEL ||
                      temperature >= TEMPERATURE_DANGER_C;
  const bool warning = gasValue >= GAS_WARNING_LEVEL ||
                       temperature >= TEMPERATURE_WARNING_C;

  if (danger) {
    showDanger();
  } else if (warning) {
    showWarning();
  } else {
    showNormal();
  }

  Serial.print(F("Gas raw: "));
  Serial.print(gasValue);
  Serial.print(F(" | Temperature: "));
  Serial.print(temperature);
  Serial.print(F(" C | Humidity: "));
  Serial.print(humidity);
  Serial.println(F(" %"));

  delay(2000);
}
