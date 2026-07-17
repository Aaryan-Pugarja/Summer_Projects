#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define LED_PIN 2

// Custom UUIDs
#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890AB"
#define CHARACTERISTIC_UUID "87654321-4321-4321-4321-BA0987654321"

BLECharacteristic* pCharacteristic;

class CommandCallback : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic* characteristic) override
    {
        String command = characteristic->getValue().c_str();
        command.trim();
        command.toUpperCase();

        Serial.print("Received: ");
        Serial.println(command);

        if (command == "ON")
        {
            digitalWrite(LED_PIN, HIGH);
            Serial.println("LED ON");
        }
        else if (command == "OFF")
        {
            digitalWrite(LED_PIN, LOW);
            Serial.println("LED OFF");
        }
        else
        {
            Serial.println("Unknown command");
        }
    }
};

void setup()
{
    Serial.begin(115200);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    BLEDevice::init("ESP32 Light Controller");

    BLEServer* server = BLEDevice::createServer();

    BLEService* service = server->createService(SERVICE_UUID);

    pCharacteristic = service->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );

    pCharacteristic->setCallbacks(new CommandCallback());

    service->start();

    BLEAdvertising* advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(SERVICE_UUID);
    advertising->start();

    Serial.println("BLE Server Started");
    Serial.println("Waiting for connection...");
}

void loop()
{
    delay(100);
}
