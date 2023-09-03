

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



//Mac Inside
uint8_t revieverMAC[] = {0x48, 0xE7, 0x29, 0x9E, 0xB4, 0xB8};

typedef struct broadcastMessage {
  bool b;
} broadcastMessage;

broadcastMessage message; // message to send

esp_now_peer_info_t peerInfo; // required by espNow


// Network credentials
const char* ssid = ":)";
const char* password = ":)";


// Initialize Telegram BOT
#define BOTtoken ":)"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID ":)"
#define motionSensor GPIO_NUM_12 // PIR Motion Sensor

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

bool motionDetected = false;
void goToDeepSleep()
{
  Serial.println("Going to sleep...");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  btStop();

  adc_power_off();
  esp_wifi_stop();
  esp_bt_controller_disable();

  // Go to sleep! Zzzz
  esp_deep_sleep_start();
}

// Indicates when motion is detected
void IRAM_ATTR detectsMovement() {
  motionDetected = true;
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {

  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  Serial.begin(115200);
  Serial.println(WiFi.macAddress());

  pinMode(motionSensor, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, FALLING);

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
  
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, revieverMAC, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer

  Serial.println("The one below me");
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  bot.sendMessage(CHAT_ID, "Hooters woke up! \xF0\x9F\xA6\x89. \xF0\x9F\x92\xA4 ", "");

  esp_sleep_enable_ext0_wakeup(motionSensor,1); // Connect External deep sleep wake up to PIR pin
  
}

void loop() {
  
  if(motionDetected){

    //bot.sendMessage(CHAT_ID, "Motion detected!! \xF0\x9F\x91\x80", "");
    Serial.println("Motion Detected, Creating a message to send to the reviever!");
    // Set values to send
    message.b = false;
    Serial.println(message.b);
    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(revieverMAC, (uint8_t *) &message, sizeof(message));
    if (result == ESP_OK) {
      Serial.println(result);
      Serial.println("Sent with success");
    }
    else {
      Serial.println(result);
      Serial.println("Error sending the data");
    }
    Serial.println("Setting MotionDetected to false");
    motionDetected = false;
    goToDeepSleep();
  }
}
