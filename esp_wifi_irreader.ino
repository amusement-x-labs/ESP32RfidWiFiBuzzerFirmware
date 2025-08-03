#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>
#include <MFRC522Debug.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "pitches.h"

#define BUZZER_PIN  16

#define LED_ONBOARD 2
#define LED 4

MFRC522DriverPinSimple ss_pin(5);
MFRC522DriverSPI driver{ss_pin};
MFRC522 mfrc522{driver};

// The Imperial March /Darth Vader's Theme (Star Wars)
int melody1[] = {
  NOTE_A4, NOTE_A4, NOTE_A4, NOTE_F4, NOTE_C5,
  NOTE_A4, NOTE_F4, NOTE_C5, NOTE_A4, 
  NOTE_E5, NOTE_E5, NOTE_E5, NOTE_F5, NOTE_C5,
  NOTE_GS4, NOTE_F4, NOTE_C5, NOTE_A4
};
int noteDurations1[] = {
  4, 4, 4, 5, 16,
  4, 5, 16, 2,
  4, 4, 4, 5, 16,
  4, 5, 16, 2
};

// Start of the Main Theme in Pirates of the Caribbean
int melody2[] = {
  NOTE_E4, NOTE_G4, NOTE_A4, NOTE_A4, 0,
  NOTE_A4, NOTE_B4, NOTE_C5, NOTE_C5, 0,
  NOTE_C5, NOTE_D5, NOTE_B4, NOTE_B4, 0,
  NOTE_A4, NOTE_G4, NOTE_A4, 0
};
int noteDurations2[] = {
  8, 8, 4, 8, 8,
  8, 8, 4, 8, 8,
  8, 8, 4, 8, 8,
  8, 8, 4, 8
};

bool isIn = false;
String authToken = "";

const char* ssid     = "WIFI_SID"; // setup you WiFi name
const char* password = "WIFI_PASSWORD"; // setup your password
const char* web_server_host = "192.168.100.10";
const uint16_t web_server_port = 5000;

const char* server_username = "admin";
const char* server_password = "password123";

const int MAX_WIFI_ATTEMPTS_COUNT = 20;

void playSoundEffect(int melody[], int noteDurations[], int size) {
  noTone(BUZZER_PIN);
  
  for (int thisNote = 0; thisNote < size; thisNote++) {
    if (melody[thisNote] == 0) {
      delay(1000 / noteDurations[thisNote]); // Pause
      continue;
    }

    tone(BUZZER_PIN, melody[thisNote]);
    delay(1000 / noteDurations[thisNote]);
    noTone(BUZZER_PIN);
    delay(30);
  }
}

bool loginToServer() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected, cannot login");
    return false;
  }

  HTTPClient http;
  String url = "http://" + String(web_server_host) + ":" + String(web_server_port) + "/login";
  
  // Create JSON payload
  DynamicJsonDocument doc(200);
  doc["username"] = server_username;
  doc["password"] = server_password;
  String payload;
  serializeJson(doc, payload);

  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  
  int httpCode = http.POST(payload);
  
  if (httpCode == HTTP_CODE_OK) {
    String response = http.getString();
    DynamicJsonDocument responseDoc(200);
    deserializeJson(responseDoc, response);
    
    authToken = responseDoc["token"].as<String>();
    Serial.println("Login successful. Token received: " + authToken);
    http.end();
    return true;
  } else {
    Serial.print("Login failed. HTTP code: ");
    Serial.println(httpCode);
    http.end();
    return false;
  }
}

bool sendStatusUpdate(bool status) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected");
    return false;
  }

  HTTPClient http;
  String url = "http://" + String(web_server_host) + ":" + String(web_server_port) + "/statusChanged";
  
  // Create JSON payload
  DynamicJsonDocument doc(200);
  doc["status"] = status;
  String payload;
  serializeJson(doc, payload);

  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + authToken);
  
  int httpCode = http.POST(payload);
  
  if (httpCode == HTTP_CODE_OK) {
    String response = http.getString();
    Serial.print("Status update successful. Response: ");
    Serial.println(response);
    http.end();
    return true;
  } else if (httpCode == 401) { // Unauthorized - token is invalid
    Serial.println("Token invalid, attempting to re-login");
    http.end();
    
    if (loginToServer()) {
      // Try again with new token
      http.begin(url);
      http.addHeader("Content-Type", "application/json");
      http.addHeader("Authorization", "Bearer " + authToken);
      
      httpCode = http.POST(payload);
      
      if (httpCode == HTTP_CODE_OK) {
        String response = http.getString();
        Serial.print("Status update successful after re-login. Response: ");
        Serial.println(response);
        http.end();
        return true;
      } else {
        Serial.print("Status update failed after re-login. HTTP code: ");
        Serial.println(httpCode);
        http.end();
        return false;
      }
    } else {
      Serial.println("Re-login failed");
      return false;
    }
  } else {
    Serial.print("Status update failed. HTTP code: ");
    Serial.println(httpCode);
    http.end();
    return false;
  }
}

void setup() {
  pinMode(LED_ONBOARD,OUTPUT);
  pinMode(LED,OUTPUT);
  digitalWrite(LED,LOW);

  WiFi.disconnect(true, true);
  WiFi.begin(ssid, password);
  uint8_t wifiAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && wifiAttempts < MAX_WIFI_ATTEMPTS_COUNT) {
    digitalWrite(LED_ONBOARD,HIGH);
    Serial.print(".");
    delay(1000);
    digitalWrite(LED_ONBOARD,LOW);
    if(wifiAttempts == 10) {
      WiFi.disconnect(true, true);
      WiFi.begin(ssid, password);
    }
    wifiAttempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi configured");
    digitalWrite(LED_ONBOARD,HIGH);
    Serial.println(WiFi.localIP());

    // Attempt to login to server
    if (!loginToServer()) {
      Serial.println("Failed to login to server");
    }
  } else {
    WiFi.disconnect(true, true);
    Serial.print("WiFi isn't setup");
  }

  Serial.begin(115200);
  while (!Serial);
  
  mfrc522.PCD_Init();
  MFRC522Debug::PCD_DumpVersionToSerial(mfrc522, Serial);
}

void loop() {
  // Reset the loop if no new card present on the sensor/reader
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards.
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  if(!isIn) {
    isIn = true;
    digitalWrite(LED,HIGH);
    playSoundEffect(melody1, noteDurations1, sizeof(melody1)/sizeof(int));
    sendStatusUpdate(isIn);
  } else {
    isIn = false;
    digitalWrite(LED,LOW);
    playSoundEffect(melody2, noteDurations2, sizeof(melody2)/sizeof(int));
    sendStatusUpdate(isIn);
  }

  delay(1000);
}