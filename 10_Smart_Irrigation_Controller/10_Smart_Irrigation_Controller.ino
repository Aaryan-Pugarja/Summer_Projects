/*
  Smart Irrigation Controller - Arduino Uno R3

  Connections:
  Soil-moisture analog output -> A0
  Rain-sensor digital output -> D2
  Relay input -> D7

  The pump must use an external power supply suitable for the pump.
  Join the external supply ground to Arduino ground when required by the relay.
*/

const byte SOIL_PIN = A0;
const byte RAIN_PIN = 2;
const byte RELAY_PIN = 7;

const bool RELAY_ACTIVE_LOW = true;
const bool RAIN_ACTIVE_LOW = true;

const int DRY_SOIL_THRESHOLD = 650;
const int WET_SOIL_THRESHOLD = 500;

const unsigned long MAX_PUMP_RUN_MS = 15000;
const unsigned long PUMP_COOLDOWN_MS = 30000;

bool pumpRunning = false;
unsigned long pumpStartedAt = 0;
unsigned long lastPumpStoppedAt = 0;

void setPump(bool enabled) {
  pumpRunning = enabled;
  digitalWrite(RELAY_PIN, RELAY_ACTIVE_LOW ? !enabled : enabled);

  if (enabled) {
    pumpStartedAt = millis();
  } else {
    lastPumpStoppedAt = millis();
  }
}

void setup() {
  pinMode(RAIN_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);

  Serial.begin(9600);
  setPump(false);
}

void loop() {
  const int soilValue = analogRead(SOIL_PIN);
  const bool rainDetected =
      digitalRead(RAIN_PIN) == (RAIN_ACTIVE_LOW ? LOW : HIGH);

  if (pumpRunning) {
    const bool soilWetEnough = soilValue <= WET_SOIL_THRESHOLD;
    const bool protectionTimeout =
        millis() - pumpStartedAt >= MAX_PUMP_RUN_MS;

    if (rainDetected || soilWetEnough || protectionTimeout) {
      setPump(false);
    }
  } else {
    const bool soilIsDry = soilValue >= DRY_SOIL_THRESHOLD;
    const bool cooldownComplete =
        millis() - lastPumpStoppedAt >= PUMP_COOLDOWN_MS;

    if (soilIsDry && !rainDetected && cooldownComplete) {
      setPump(true);
    }
  }

  Serial.print(F("Soil raw: "));
  Serial.print(soilValue);
  Serial.print(F(" | Rain: "));
  Serial.print(rainDetected ? F("YES") : F("NO"));
  Serial.print(F(" | Pump: "));
  Serial.println(pumpRunning ? F("ON") : F("OFF"));

  delay(250);
}
