#include <ArduinoBLE.h>

#define LED_UP 12
#define LED_STOP 11
#define LED_DOWN 10
#define MAX_HEIGHT 200
#define MIN_HEIGHT 0
#define UPDATE_INTERVAL 100
#define STOP 0
#define UP 1
#define DOWN 2

// BLE services and characteristics
BLEService rollerDoorService("00000020-0000-1000-8000-00805f9b34fb"); 
BLEUnsignedIntCharacteristic heightCharacteristic("00000021-0000-1000-8000-00805f9b34fb", BLERead | BLENotify);
BLEIntCharacteristic controlCharacteristic("00000022-0000-1000-8000-00805f9b34fb", BLERead | BLEWrite);
unsigned int height = 0;
int control = STOP;

void setup() {
  pinMode(LED_UP, OUTPUT);
  pinMode(LED_STOP, OUTPUT);
  pinMode(LED_DOWN, OUTPUT);
  Serial.begin(9600);

  // BLE initialization
  if (!BLE.begin()) {
    Serial.println("BLE init failed.");
    while (1);
  }

  // BLE setup
  BLE.setLocalName("Arduino");
  BLE.setDeviceName("Arduino");
  BLE.setAdvertisedService(rollerDoorService);
  rollerDoorService.addCharacteristic(heightCharacteristic);
  rollerDoorService.addCharacteristic(controlCharacteristic);
  BLE.addService(rollerDoorService);
  heightCharacteristic.writeValue(height);
  controlCharacteristic.writeValue(control);

  // BLE event handler
  BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

  BLE.advertise();
}

void loop() {
  BLEDevice central = BLE.central();
  if (central) {
    unsigned long previousMillis = 0;
    unsigned long currentMillis = 0;
    while (central.connected()) {
      // update height
      currentMillis = millis();
      if (currentMillis - previousMillis >= UPDATE_INTERVAL) {
        previousMillis = currentMillis;
        updateHeight();
      }
    }
  }
}

void blePeripheralConnectHandler(BLEDevice central) {
  Serial.print("[Connect] central: ");
  Serial.println(central.address());
}

void blePeripheralDisconnectHandler(BLEDevice central) {
  Serial.print("[Disconnect] central: ");
  Serial.println(central.address());
}

void updateHeight() {
  switch(control = controlCharacteristic.value()) {
    case UP: 
      height = (height + 1 <= MAX_HEIGHT) ? height + 1 : height; 
      break;
    case DOWN:
      height = ((int)height - 1 >= MIN_HEIGHT) ? height - 1 : height; // evaluates to true if height is unsigned
      break;
    default:
      break;
  }
  // write to LED
  if (height > MIN_HEIGHT && height < MAX_HEIGHT) 
    writeToLed(control);
  else
    writeToLed(STOP);
  heightCharacteristic.writeValue(height);
}

void writeToLed(int controlSignal) {
  switch(controlSignal) {
    case UP:
      digitalWrite(LED_UP, HIGH);
      digitalWrite(LED_STOP, LOW);
      digitalWrite(LED_DOWN, LOW);
      break;
    case DOWN:
      digitalWrite(LED_UP, LOW);
      digitalWrite(LED_STOP, LOW);
      digitalWrite(LED_DOWN, HIGH);
      break;
    default:
      digitalWrite(LED_UP, LOW);
      digitalWrite(LED_STOP, HIGH);
      digitalWrite(LED_DOWN, LOW);
      break;
  }
}
