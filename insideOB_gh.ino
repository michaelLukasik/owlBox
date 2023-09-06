

/*
  Rui Santos (ty rui B))
  Complete project details at https://RandomNerdTutorials.com/telegram-esp32-motion-detection-arduino/
  
  Project created using Brian Lough's Universal Telegram Bot Library: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "driver/adc.h"
#include "esp_sleep.h"
#include "driver/gpio.h"
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_bt.h>
#include <HTTPClient.h>
#include <time.h>

const long  gmtOffset_sec = 19800;
const int   daylightOffset_sec = 0;


typedef struct broadcastMessage {
    float temperature;
    float humidity;
} broadcastMessage;



broadcastMessage message;

esp_now_peer_info_t peerInfo; // required by espNow

// Network credentials
const char* ssid = "xxx";
const char* password = "xxx";
String GOOGLE_SCRIPT_ID = "xxx";

WiFiClientSecure client;

const int lightRelay = 27; // Light Relay 

// callback when data is recieved
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  Serial.println("Entered OnDataRecv");
  memcpy(&message, incomingData, sizeof(message));
  Serial.println(WiFi.status());
  Serial.println("--Message received--");
  Serial.println("Temperature: ");
  Serial.println(message.temperature);
  Serial.println("Humidity: ");
  Serial.println(message.humidity);
  WiFi.printDiag(Serial);

  // Activate the Light for our viewing pleasure 
  Serial.println("Setting the relay at pin 12 on for 5 seconds");
  digitalWrite(lightRelay, HIGH); 
  delay(100);
  digitalWrite(lightRelay, LOW);
  delay(100);
  digitalWrite(lightRelay, HIGH); 
  delay(5000);
  digitalWrite(lightRelay, LOW); 
}


int32_t getWiFiChannel(const char *ssid) {
  if (int32_t n = WiFi.scanNetworks()) {
      for (uint8_t i=0; i<n; i++) {
          if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
              return WiFi.channel(i);
          }
      }
  }
  return 0;
}




void setup() {
  Serial.begin(115200);
  Serial.println(WiFi.macAddress());

  pinMode(lightRelay, OUTPUT);

  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  int32_t channel = getWiFiChannel(ssid);
  WiFi.printDiag(Serial); // Uncomment to verify channel number before
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
  WiFi.printDiag(Serial); // Uncomment to verify channel change after
  Serial.println(" initializing ESP-NOW");
  
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("ESPNOW Call Back Registered");
}

void loop() {
}
