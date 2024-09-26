#include <ArduinoBLE.h>

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214");
BLEByteCharacteristic ledCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

const int ledPin = LED_BUILTIN;

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);

  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
    while (1);
  }

  BLE.setLocalName("NanoBLE_LED"); 
  BLE.setAdvertisedService(ledService);
  ledService.addCharacteristic(ledCharacteristic);
  BLE.addService(ledService);
  
  ledCharacteristic.writeValue(0); 
  BLE.advertise();
  
  Serial.println("Bluetooth device active, waiting for connections...");
}

void loop() {
  BLEDevice central = BLE.central(); 

  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    while (central.connected()) {
      if (ledCharacteristic.written()) {
        int ledState = ledCharacteristic.value();
        digitalWrite(ledPin, ledState);
      }
    }

    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}
