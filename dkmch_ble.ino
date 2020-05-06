#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define SERVICE_UUID "7067452c-0513-41a0-a0bd-b8582a217bb0"
#define CHARACTERISTIC_UUID "0000FFE1-0000-1000-8000-00805F9B34FB" // xcsoar mandates this (BluetoothGattClientPort.java)

SemaphoreHandle_t ticks_semaphore;
int ticks = 0;

uint8_t nmea_checksum(const char *line) {
  // XORs everything between $ and *
  if (line[0] == '$') {
    line++;
  }

  uint8_t cs = 0;
  for (int i = 0; i < strlen(line); i++) {
    cs ^= line[i];
  }

  return cs;
}

void nmea_build_flo(char *buf, double val) {
  // $PFLO,xxx.x8??
  sprintf(buf, "$PFLO,%#05.1f", val);
  uint8_t cs = nmea_checksum(buf);
  sprintf(buf+strlen(buf), "*%2X", cs);
  strcat(buf, "\n");
}

void TaskBLE(void *params) {
  (void) params;

  BLEDevice::init("D-KMCH");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_NOTIFY |
    BLECharacteristic::PROPERTY_INDICATE
  );
  pCharacteristic->addDescriptor(new BLE2902());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();

  while (ticks_semaphore == NULL) {};

  for (;;) {
    char buf[128];
    int ticks_local = 0;

    while (xSemaphoreTake(ticks_semaphore, 0) == pdFALSE) {};
    ticks_local = ticks;
    ticks = 0;
    xSemaphoreGive(ticks_semaphore);

    nmea_build_flo(buf, ticks_local);

    pCharacteristic->setValue(buf);
    pCharacteristic->notify();

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void TaskPoll(void *params) {
  (void) params;

  ticks_semaphore = xSemaphoreCreateMutex();

  for (;;) {
    while (xSemaphoreTake(ticks_semaphore, 0) == pdFALSE) {};
    ticks++;
    xSemaphoreGive(ticks_semaphore);

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void setup() {
  xTaskCreatePinnedToCore(
    TaskPoll,
    "TaskPoll",
    1024,
    NULL,
    1,
    NULL,
    1
  );
  xTaskCreatePinnedToCore(
    TaskBLE,
    "TaskBLE",
    10240,
    NULL,
    2,
    NULL,
    1
  );
}

void loop() {
  // put your main code here, to run repeatedly:

}
