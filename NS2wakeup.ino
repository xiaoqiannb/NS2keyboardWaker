#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <esp_mac.h>

#define DEVICE_NAME "NS2_Wake"
#define BUTTON_PIN 14  // 更改引脚

BLEAdvertising *pAdvertising;
bool bleInitialized = false;

// 指定的MAC地址

/*
!!!!!!!!!!!!!!!!!!!!
mac地址要设置成蓝牙抓包抓到的mac地址的最后一位十六进制减二，这个esp32会偏移！！！！！！！！
*/
const uint8_t TARGET_MAC[6] = {0x78, 0x81, 0x8c, 0x00, 0x5a, 0x29};

// 广播数据
const uint8_t TARGET_ADV_DATA[] = {
  0x02, 0x01, 0x06, 0x1B, 0xFF, 0x53, 0x05, 0x01, 0x00, 0x03, 0x7E, 0x05, 0x66, 
  0x20, 0x00, 0x01, 0x81, 0xBB, 0x72, 0x11, 0x8C, 0x81, 0x78, 0x0F, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00
};
const size_t ADV_DATA_SIZE = sizeof(TARGET_ADV_DATA);

void initBLE() {
  Serial.println("初始化BLE...");
  
  esp_base_mac_addr_set((uint8_t*)TARGET_MAC);
  BLEDevice::init(DEVICE_NAME);
  
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_BT);
  Serial.printf("BLE MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", 
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->setMinInterval(0x0020);
  pAdvertising->setMaxInterval(0x0040);
  pAdvertising->setAdvertisementType(ADV_TYPE_NONCONN_IND);

  bleInitialized = true;
}

void sendBLEAdvertisement() {
  Serial.println("发送NS2唤醒信号...");
  
  BLEAdvertisementData advertisementData;
  String rawData = "";
  for (size_t i = 0; i < ADV_DATA_SIZE; i++) {
    rawData += (char)TARGET_ADV_DATA[i];
  }
  
  advertisementData.addData(rawData);
  pAdvertising->setAdvertisementData(advertisementData);
  
  pAdvertising->start();
  Serial.println("NS2唤醒信号已发送 (10秒持续时间)");
  
  delay(10000);
  
  pAdvertising->stop();
  Serial.println("信号发送完成");
}

void setup() {
  Serial.begin(115200);
  delay(500);
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.print("按钮引脚设置: GPIO");
  Serial.print(BUTTON_PIN);
  
  initBLE();
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    Serial.println("检测到按钮操作!");
    sendBLEAdvertisement();
    // 等待按钮释放
    while (digitalRead(BUTTON_PIN) == LOW) {
      delay(10);
    }
    Serial.println("等待下一次唤醒...");
  }
  delay(50);
}