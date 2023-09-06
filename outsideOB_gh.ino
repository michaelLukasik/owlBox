

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
#include <HTTPClient.h>

#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_bt.h>
#include "DHT.h"



//Mac Inside
uint8_t revieverMAC[] = {0x48, 0xE7, 0x29, 0x9E, 0xB4, 0xB8};

typedef struct broadcastMessage {
  float temperature;
  float humidity;
} broadcastMessage;

broadcastMessage message; // message to send

esp_now_peer_info_t peerInfo; // required by espNow


// Network credentials
const char* ssid = "xxx";
const char* password = "xxx";
String GOOGLE_SCRIPT_ID = "xxx";

// Initialize Telegram BOT
#define BOTtoken "xxx"  // your Bot Token (Get from Botfather)
#define CHAT_ID "xxx"

#define motionSensor GPIO_NUM_27 // PIR Motion Sensor
#define DHTPIN 26  // Temp/Humidity Sensor 
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

bool motionDetected = false;
float t = -100.; // Strangly need to initialize to some values, can't assign on the spot.
float h = -0.1;

void goToDeepSleep(){
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

// Helper Function to get the correct channel
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
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

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
  int readData = dht.read(DHTPIN); // Reads the data from the sensor
  t = dht.readTemperature(true); // Gets the values of the temperature
  h = dht.readHumidity(); // Gets the values of the humidity
  
  // Get environment information from the collection ESP, save into google sheets.
  String urlFinal = "https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?"+"&temperature=" + String(t)+ "&humidity=" + String(h);
  Serial.print("Post data to spreadsheet: ");
  Serial.println(urlFinal);
  Serial.println(t);
  Serial.println(h);

    
  HTTPClient http;
  http.begin(urlFinal.c_str());
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.GET(); 
  Serial.print("HTTP Status Code: ");
  Serial.println(httpCode);
  //Check response from google sheets
  String payload;
  if (httpCode > 0) {
      payload = http.getString();
      Serial.println("Payload: "+payload);    
  }
  http.end();
  

  
  bot.sendMessage(CHAT_ID, "Hooters woke up! \xF0\x9F\xA6\x89. \xF0\x9F\x92\xA4 ", "");
  WiFi.disconnect();
  
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
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
     
  esp_sleep_enable_ext0_wakeup(motionSensor,1); // Connect External deep sleep wake up to PIR pin
}

void loop() {
  
  if(motionDetected){
    
    Serial.println("Motion Detected, Creating a message to send to the reviever!");
    
    // Set values to send
    message.temperature = dht.readTemperature();
    message.humidity = dht.readHumidity();
    Serial.println("Sending a packet with Temperature/Humidity of:");

    
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
    goToDeepSleep(); // Careful with this
  }
}
