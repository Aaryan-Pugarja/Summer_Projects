/*
  Project 10: Mini Security Alarm

  Components:
  - Arduino UNO
  - HC-SR505 Mini PIR Motion Sensor
  - 3V Buzzer
  - Red LED

  Connections:
  HC-SR505 VCC  -> 5V
  HC-SR505 GND  -> GND
  HC-SR505 OUT  -> Digital pin 2

  Buzzer +      -> Digital pin 8
  Buzzer -      -> GND

  Red LED anode -> Digital pin 9 through a 220 ohm resistor
  Red LED cathode -> GND
*/

const byte PIR_PIN = 2;
const byte BUZZER_PIN = 8;
const byte LED_PIN = 9;

const unsigned long ALARM_DURATION_MS = 5000;

unsigned long alarmStartedAt = 0;
bool alarmActive = false;

void setup() {
  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  Serial.begin(9600);
  Serial.println("Mini Security Alarm");
  Serial.println("Allow the PIR sensor a few seconds to stabilize.");
}

void loop() {
  bool motionDetected = digitalRead(PIR_PIN) == HIGH;

  if (motionDetected) {
    alarmActive = true;
    alarmStartedAt = millis();

    Serial.println("Motion detected!");
  }

  if (alarmActive) {
    digitalWrite(LED_PIN, HIGH);
    tone(BUZZER_PIN, 1000);

    if (millis() - alarmStartedAt >= ALARM_DURATION_MS) {
      alarmActive = false;
      digitalWrite(LED_PIN, LOW);
      noTone(BUZZER_PIN);
    }
  } else {
    digitalWrite(LED_PIN, LOW);
    noTone(BUZZER_PIN);
  }

  delay(50);
}
