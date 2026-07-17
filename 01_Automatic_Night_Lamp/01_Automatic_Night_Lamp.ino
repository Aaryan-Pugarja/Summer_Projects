/*
  Automatic Night Lamp - Arduino Uno R3

  Connections:
  LDR divider output -> A0
  LED through 220 ohm resistor -> D9
  LDR divider: 5V -- LDR -- A0 -- 10k resistor -- GND

  Adjust LIGHT_ON_THRESHOLD after checking the Serial Monitor.
*/

const byte LDR_PIN = A0;
const byte LED_PIN = 9;

const int LIGHT_ON_THRESHOLD = 400;
const int HYSTERESIS = 30;

bool lampOn = false;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  const int lightValue = analogRead(LDR_PIN);

  if (!lampOn && lightValue < LIGHT_ON_THRESHOLD) {
    lampOn = true;
  } else if (lampOn && lightValue > LIGHT_ON_THRESHOLD + HYSTERESIS) {
    lampOn = false;
  }

  digitalWrite(LED_PIN, lampOn ? HIGH : LOW);

  Serial.print(F("Light reading: "));
  Serial.print(lightValue);
  Serial.print(F(" | Lamp: "));
  Serial.println(lampOn ? F("ON") : F("OFF"));

  delay(100);
}
