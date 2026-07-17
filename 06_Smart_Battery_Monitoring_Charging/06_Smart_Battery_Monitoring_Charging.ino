#include <Wire.h>
#include <Adafruit_INA219.h>
#include <Adafruit_MAX1704X.h>
#include <EEPROM.h>

/*
  Smart Battery Monitoring and Intelligent Charging System

  Install:
  - Adafruit INA219
  - Adafruit MAX1704X

  Connections:
  INA219 and MAX17048 use I2C: SDA -> A4, SCL -> A5
  Relay input -> D7
  Buzzer -> D8
  Red LED -> D9
  Blue LED -> D10
  Green LED -> D11

  Important:
  - The TP4056 is designed for one 3.7V Li-ion cell, not two cells in series.
  - This sketch assumes a properly protected single-cell battery system.
  - The relay controls the charger input or another correctly designed charging
    control path. Do not switch battery current without suitable hardware.
  - Never power a Li-ion cell directly from the Arduino.
*/

Adafruit_INA219 ina219;
Adafruit_MAX17048 fuelGauge;

const byte RELAY_PIN = 7;
const byte BUZZER_PIN = 8;
const byte RED_LED_PIN = 9;
const byte BLUE_LED_PIN = 10;
const byte GREEN_LED_PIN = 11;

const bool RELAY_ACTIVE_LOW = true;
const float START_CHARGING_SOC = 25.0;
const float STOP_CHARGING_SOC = 90.0;
const float OVERCURRENT_MA = 1000.0;
const float BATTERY_PRESENT_V = 2.8;
const float CHARGER_DETECTION_MA = 20.0;

const int EEPROM_CYCLE_ADDRESS = 0;
bool chargingEnabled = false;
bool wasBelowStartThreshold = false;
unsigned int cycleCount = 0;

void setRelay(bool enabled) {
  chargingEnabled = enabled;
  digitalWrite(
      RELAY_PIN,
      RELAY_ACTIVE_LOW ? !enabled : enabled
  );
}

void loadCycleCount() {
  EEPROM.get(EEPROM_CYCLE_ADDRESS, cycleCount);
  if (cycleCount == 0xFFFF) cycleCount = 0;
}

void saveCycleCount() {
  EEPROM.put(EEPROM_CYCLE_ADDRESS, cycleCount);
}

void setIndicators(bool fault, bool charging) {
  digitalWrite(RED_LED_PIN, fault);
  digitalWrite(BLUE_LED_PIN, !fault && charging);
  digitalWrite(GREEN_LED_PIN, !fault && !charging);
}

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);

  Serial.begin(9600);
  Wire.begin();
  loadCycleCount();
  setRelay(false);

  if (!ina219.begin()) {
    Serial.println(F("INA219 not found."));
    while (true) {}
  }

  if (!fuelGauge.begin()) {
    Serial.println(F("MAX17048 not found."));
    while (true) {}
  }
}

void loop() {
  const float batteryVoltage = fuelGauge.cellVoltage();
  const float stateOfCharge = fuelGauge.cellPercent();
  const float busVoltage = ina219.getBusVoltage_V();
  const float currentMa = ina219.getCurrent_mA();
  const float powerMw = ina219.getPower_mW();

  const bool batteryPresent = batteryVoltage >= BATTERY_PRESENT_V;
  const bool overcurrent = abs(currentMa) >= OVERCURRENT_MA;
  const bool chargerLikelyPresent = currentMa >= CHARGER_DETECTION_MA;

  if (!batteryPresent || overcurrent) {
    setRelay(false);
    setIndicators(true, false);
    tone(BUZZER_PIN, 2200);
  } else {
    noTone(BUZZER_PIN);

    if (stateOfCharge <= START_CHARGING_SOC) {
      wasBelowStartThreshold = true;
      setRelay(true);
    }

    if (stateOfCharge >= STOP_CHARGING_SOC) {
      if (chargingEnabled && wasBelowStartThreshold) {
        cycleCount++;
        saveCycleCount();
        wasBelowStartThreshold = false;
      }
      setRelay(false);
    }

    setIndicators(false, chargingEnabled);
  }

  float estimatedHours = -1.0;
  if (currentMa < -10.0) {
    const float assumedCapacityMah = 2000.0;
    estimatedHours = assumedCapacityMah * stateOfCharge / 100.0 / abs(currentMa);
  }

  Serial.print(F("Battery: "));
  Serial.print(batteryVoltage, 3);
  Serial.print(F(" V | SoC: "));
  Serial.print(stateOfCharge, 1);
  Serial.print(F("% | Current: "));
  Serial.print(currentMa, 1);
  Serial.print(F(" mA | Power: "));
  Serial.print(powerMw, 1);
  Serial.print(F(" mW | Bus: "));
  Serial.print(busVoltage, 2);
  Serial.print(F(" V | Charger detected: "));
  Serial.print(chargerLikelyPresent ? F("YES") : F("NO"));
  Serial.print(F(" | Cycles: "));
  Serial.print(cycleCount);

  if (estimatedHours >= 0) {
    Serial.print(F(" | Runtime estimate: "));
    Serial.print(estimatedHours, 1);
    Serial.print(F(" h"));
  }

  Serial.println();
  delay(1000);
}
