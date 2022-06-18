/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini

   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
   Has a characteristic of: 6E400002-B5A3-F393-E0A9-E50E24DCCA9E - used for receiving data with "WRITE"
   Has a characteristic of: 6E400003-B5A3-F393-E0A9-E50E24DCCA9E - used to send data with  "NOTIFY"

   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   In this example rxValue is the data received (only accessible inside that function).
   And txValue is the data to be sent, in this example just a byte incremented every second.
*/
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "esp_log.h"

BLEServer *pServer = NULL;

BLECharacteristic *pTxCharacteristic;

bool deviceConnected = false;

bool oldDeviceConnected = false;

uint8_t txValue = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

unsigned long pTime = 0UL;
int xcr[3];
int ycr[3];

void xy_coordination(int x, int y)
{
  int xc = 200, yc = 150, curX =0, curY =0; 

  if (x > xc)
  {
    curX = x - xc; 
  }    
  else{
    curX = x;

  }

  if (y > yc)
  {
    curY = y - yc;

  }
  else{
    curY = y; 
  }

   Serial.printf(" curX : %d -> curY : %d\n", curX, curY);
   // your codes 
   // left movement/ rgt. front backword 




}


class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer *pServer)
  {
    deviceConnected = false;
  }
};

class MyCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0)
    {

      int flgx = 0;
      int flgy = 0;

      Serial.println("*********");
      Serial.print("Received Value: ");

      int tmp = 0;
      int tmp2 = 0;
      int j = 0;
      int k = 0;
      for (int i = 0; i < rxValue.length(); i++)
      {
        if (rxValue[i] != '\r' && rxValue[i] != '\n' && rxValue[i] != '\0')
        {
          // Serial.print(rxValue[i]);
          if (rxValue[i] == '$')
          {
            tmp = i + 1;

            while (rxValue[tmp] != ':')
            {
              xcr[j] = rxValue[tmp] - '0';
              Serial.printf("x value : %d ", xcr[j]);
              j++;
              tmp++;
            }
          }
          if (rxValue[i] == ':')
          {
            tmp2 = i + 1;

            while (rxValue[tmp2] != '#')
            {
              ycr[k] = rxValue[tmp2] - '0';
              Serial.printf("y value : %d ", ycr[k]);
              k++;
              tmp2++;
            }
          }   if (curY > 20)
   {
    digitalWrite(LED, HIGH); 
   }
      }
      int x = 0;
      Serial.printf("k value : %d ", k);
      Serial.printf("j value : %d ", j);
      switch (j)
      {
      case 1:
        x = xcr[0];
        break;
      case 2:
        x = xcr[0] * 10 + xcr[1];
        break;
      case 3:
        x = xcr[0] * 100 + xcr[1] * 10 + xcr[2];
        break;
      }

      int y = 0;
      switch (k)
      {
      case 1:
        y = ycr[0];
        break;
      case 2:
        y = ycr[0] * 10 + ycr[1];
        break;
      case 3:
        y = ycr[0] * 100 + ycr[1] * 10 + ycr[2];
        break;
      }
      Serial.printf(" X : %d -> Y : %d\n", x, y);
      xy_coordination(x, y);
    }
  }
};

void setup()
{
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init("LION:BIT BLE"); // change the name 

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_TX,
      BLECharacteristic::PROPERTY_NOTIFY |
          BLECharacteristic::PROPERTY_READ);

  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_RX,
      BLECharacteristic::PROPERTY_WRITE);

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void loop()
{

  if (deviceConnected && (millis() - pTime > 1000UL))
  {
    pTxCharacteristic->setValue(&txValue, 1);
    pTxCharacteristic->notify();
    txValue++;

    pTime = millis();
  }

  // disconnecting
  if (!deviceConnected && oldDeviceConnected)
  {
    delay(500);                  // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected)
  {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }
}
