/*
 * FIRMWARE FOR COMPANION (BLE CLIENT)
 * Version 1.1 - Compilation fix
 */

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <TFT_eSPI.h>

// UUID of the service we are looking for (must match the server!)
static BLEUUID serviceUUID("199a0819-686c-4338-a268-e413028d1323");

static boolean doConnect = false;
static boolean connected = false;
static BLEAdvertisedDevice* myDevice;

TFT_eSPI tft = TFT_eSPI();

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("Disconnected!");
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("Disconnected!", tft.width() / 2, tft.height() / 2);
    tft.drawString("Restarting scan...", tft.width() / 2, tft.height() / 2 + 30);
  }
};


// Function called when the scanner finds a device
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // Check if this is our Logger
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      Serial.println("Found our Logger! Ready to connect.");
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_WHITE);
      tft.drawString("Logger found!", tft.width() / 2, tft.height() / 2);
    }
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE Client (Companion)...");

  tft.init();
  tft.setRotation(1);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2);
  
  BLEDevice::init("");
}

void loop() {
  if (doConnect == true) {
    // Create client and connect
    BLEClient* pClient = BLEDevice::createClient();
    pClient->setClientCallbacks(new MyClientCallback()); // <<< FIXED HERE
    pClient->connect(myDevice);
    Serial.println(" - Connected to server");
    tft.fillScreen(TFT_GREEN);
    tft.setTextColor(TFT_BLACK);
    tft.drawString("CONNECTED!", tft.width() / 2, tft.height() / 2);
    
    connected = true;
    doConnect = false; // Reset flag
    
    // Wait 5 seconds and disconnect (for testing)
    delay(5000);
    pClient->disconnect();

  } else if (!connected) {
    // If not connected, start scanning
    Serial.println("Scanning for Logger...");
    tft.fillScreen(TFT_BLUE);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("Scanning...", tft.width() / 2, tft.height() / 2);

    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(5, false); // Scan for 5 seconds
  }

  delay(1000);
}
