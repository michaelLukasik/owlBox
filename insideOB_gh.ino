

/*
  Rui Santos (ty rui B))
  Complete project details at https://RandomNerdTutorials.com/telegram-esp32-motion-detection-arduino/
  
  Project created using Brian Lough's Universal Telegram Bot Library: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "driver/adc.h"
#include "esp_sleep.h"
#include "driver/gpio.h"

#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_bt.h>

// Structure example to receive data
// Must match the sender structure
typedef struct broadcastMessage {
    bool b;
} broadcastMessage;

broadcastMessage message;

esp_now_peer_info_t peerInfo; // required by espNow

// Network credentials
const char* ssid = ":)";
const char* password = ":)";

// Initialize Telegram BOT
#define BOTtoken ":)"  // your Bot Token (Get from Botfather)

#define CHAT_ID ":)"


WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

const int lightRelay = 12; // Light Relay 

// callback when data is recieved
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&message, incomingData, sizeof(message));
  Serial.print("Bytes received: ");
  Serial.print("Bool: ");
  Serial.println(message.b);
  Serial.println("Setting the relay at pin 12 on for 5 seconds");
  digitalWrite(lightRelay, HIGH); 
  delay(5000);
  digitalWrite(lightRelay, LOW); 
}

void setup() {
  Serial.begin(115200);
  Serial.println(WiFi.macAddress());

  pinMode(lightRelay, OUTPUT);

  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  WiFi.disconnect();
  
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
  
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {

}
