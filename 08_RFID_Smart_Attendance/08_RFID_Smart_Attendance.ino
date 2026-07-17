#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>

/*
  RFID-Based Smart Attendance System - Arduino Uno R3

  Install: MFRC522 library by GithubCommunity/miguelbalboa.

  MFRC522:
  SDA/SS -> D10
  SCK -> D13
  MOSI -> D11
  MISO -> D12
  RST -> D9
  3.3V -> 3.3V (never 5V)

  IR proximity output -> D2
  Buzzer -> D3
  Red LED -> D4
  Blue LED -> D5
  Green LED -> D6

  Replace AUTHORIZED_UIDS with your own card UIDs shown in Serial Monitor.
*/

const byte SS_PIN = 10;
const byte RST_PIN = 9;
const byte IR_PIN = 2;
const byte BUZZER_PIN = 3;
const byte RED_LED_PIN = 4;
const byte BLUE_LED_PIN = 5;
const byte GREEN_LED_PIN = 6;

const unsigned long ENTRY_TIMEOUT_MS = 7000;
const unsigned long DUPLICATE_LOCKOUT_MS = 30000;

MFRC522 rfid(SS_PIN, RST_PIN);

const char *AUTHORIZED_UIDS[] = {
  "DEADBEEF",
  "12345678"
};
const byte AUTHORIZED_COUNT =
    sizeof(AUTHORIZED_UIDS) / sizeof(AUTHORIZED_UIDS[0]);

struct AttendanceRecord {
  byte uid[4];
  unsigned long marker;
};

const int RECORD_SIZE = sizeof(AttendanceRecord);
const int MAX_RECORDS = (EEPROM.length() - 1) / RECORD_SIZE;

String uidToString(const MFRC522::Uid &uid) {
  String result;
  for (byte i = 0; i < uid.size; i++) {
    if (uid.uidByte[i] < 0x10) result += '0';
    result += String(uid.uidByte[i], HEX);
  }
  result.toUpperCase();
  return result;
}

bool isAuthorized(const String &uid) {
  for (byte i = 0; i < AUTHORIZED_COUNT; i++) {
    if (uid == AUTHORIZED_UIDS[i]) return true;
  }
  return false;
}

bool waitForSingleEntry() {
  const unsigned long startedAt = millis();
  bool detected = false;

  while (millis() - startedAt < ENTRY_TIMEOUT_MS) {
    const bool personPresent = digitalRead(IR_PIN) == LOW;

    if (personPresent && !detected) {
      detected = true;
      delay(500);

      if (digitalRead(IR_PIN) == LOW) {
        return true;
      }
    }

    delay(20);
  }

  return false;
}

void saveAttendance(const MFRC522::Uid &uid) {
  byte count = EEPROM.read(0);
  if (count == 0xFF || count >= MAX_RECORDS) count = 0;

  AttendanceRecord record = {};
  for (byte i = 0; i < 4 && i < uid.size; i++) {
    record.uid[i] = uid.uidByte[i];
  }
  record.marker = millis();

  EEPROM.put(1 + count * RECORD_SIZE, record);
  EEPROM.update(0, count + 1);
}

void showDenied() {
  digitalWrite(RED_LED_PIN, HIGH);
  tone(BUZZER_PIN, 600, 700);
  delay(800);
  digitalWrite(RED_LED_PIN, LOW);
}

void showAccepted() {
  digitalWrite(GREEN_LED_PIN, HIGH);
  tone(BUZZER_PIN, 1800, 150);
  delay(600);
  digitalWrite(GREEN_LED_PIN, LOW);
}

void setup() {
  pinMode(IR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);

  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  Serial.println(F("Scan a card."));
}

void loop() {
  static String lastUid;
  static unsigned long lastAcceptedAt = 0;

  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  const String uid = uidToString(rfid.uid);
  Serial.print(F("UID: "));
  Serial.println(uid);

  if (!isAuthorized(uid)) {
    showDenied();
  } else if (uid == lastUid &&
             millis() - lastAcceptedAt < DUPLICATE_LOCKOUT_MS) {
    Serial.println(F("Duplicate attendance blocked."));
    showDenied();
  } else {
    digitalWrite(BLUE_LED_PIN, HIGH);
    Serial.println(F("Card accepted. Waiting for one entry..."));

    if (waitForSingleEntry()) {
      saveAttendance(rfid.uid);
      lastUid = uid;
      lastAcceptedAt = millis();
      showAccepted();
      Serial.println(F("Attendance recorded."));
    } else {
      showDenied();
      Serial.println(F("Entry was not verified."));
    }

    digitalWrite(BLUE_LED_PIN, LOW);
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(500);
}
