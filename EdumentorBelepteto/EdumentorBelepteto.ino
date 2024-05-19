#include <SPI.h>
#include <MFRC522.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "WiFi";
const char* password = "asd";

String serverName = "http://192.168.0.28:10000/";

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


#define SS_PIN 5
#define RST_PIN 0
 
MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key; 
byte nuidPICC[4];

const int red_Pin = 12;
const int green_Pin = 13;

void setup() { 
  Serial.begin(115200);

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Kijelzo allokacio hiba"));
    for(;;);
  }

  display.display();
  WiFi.begin(ssid, password);
  Serial.println("Csatlakozas");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Csatlakoztatva WiFi-re, IP: ");
  Serial.println(WiFi.localIP());  

  delay(2000);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("EduMentor");

  display.display();
  delay(2000);

  SPI.begin();
  rfid.PCD_Init();

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  pinMode(red_Pin, OUTPUT);
  pinMode(green_Pin, OUTPUT);
  digitalWrite(red_Pin, LOW);	
  digitalWrite(green_Pin, LOW);	
}
 
void loop() {

  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  if ( ! rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("NEM TAMOGATOTT KARTYA");
    display.display();
    return;
  }

  
  if (rfid.uid.uidByte[0] != nuidPICC[0] ||
    rfid.uid.uidByte[1] != nuidPICC[1] || 
    rfid.uid.uidByte[2] != nuidPICC[2] || 
    rfid.uid.uidByte[3] != nuidPICC[3] ) {

    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }
   
    String card = CardUID(rfid.uid.uidByte, rfid.uid.size);
    sendRequest(card);
  }
  else {
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("MAR BE VAN LEPVE");
    display.display();
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}


void sendRequest(String uid){
  if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;
      String serverPath = serverName + "felhasznalok/" + uid;
      Serial.println(serverPath);
      http.setTimeout(10000);
      
      http.begin(serverPath.c_str());
    
      int httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        if (httpResponseCode == 403) {
          display.clearDisplay();
          display.setCursor(0,0);
          display.println("NINCS OKTATASI IDO");
          display.display();
          digitalWrite(red_Pin, HIGH);	
          delay(5000);
          digitalWrite(red_Pin, LOW);
        }
        if (httpResponseCode == 402){
          display.clearDisplay();
          display.setCursor(0,0);
          display.println("NINCS ELEG KREDIT");
          display.display();
          digitalWrite(red_Pin, HIGH);	
          delay(500);
          digitalWrite(red_Pin, LOW);
          delay(500);
          digitalWrite(red_Pin, HIGH);	
          delay(500);
          digitalWrite(red_Pin, LOW);
          delay(500);
          digitalWrite(red_Pin, HIGH);	
          delay(3000);
          digitalWrite(red_Pin, LOW);
        }
        String payload = http.getString();
        StaticJsonDocument<300> doc;
        deserializeJson(doc, payload);
        String responseUid = doc["card"].as<String>();

        String type;
        String httpRequestData;

        if (responseUid == uid) {
          http.end();
          http.setTimeout(10000);
          String serverPath = serverName + "log/" + uid;
          Serial.print(serverPath);
          http.begin(serverPath.c_str());

          display.clearDisplay();
          display.setCursor(0,0);
          display.println("NYITVA");
          display.display();
          digitalWrite(green_Pin, HIGH);
          delay(5000);
          digitalWrite(green_Pin, LOW);

          http.addHeader("Content-Type", "application/json");
          type = "0";

          String httpRequestData = "{\"card\":\"" + uid + "\",\"type\":\"" + type + "\"}";
          httpResponseCode = http.POST(httpRequestData);
        }
        else {
          http.end();
          http.setTimeout(10000);
          String serverPath = serverName + "log/" + uid;
          Serial.print(serverPath);
          http.begin(serverPath.c_str());
          display.clearDisplay();
          display.setCursor(0,0);
          display.println("TILOS");
          display.display();
          delay(5000);

          http.addHeader("Content-Type", "application/json");
          type = "1";

          String httpRequestData = "{\"card\":\"" + uid + "\",\"type\":\"" + type + "\"}";
          httpResponseCode = http.POST(httpRequestData);
        }
      }
      else {
        Serial.print("HTTP Kod: ");
        Serial.println(httpResponseCode);
      }
      http.end();
      display.clearDisplay();
      display.setCursor(0,0);
      display.println("EduMentor");
      display.display();
    }
    else {
      Serial.println("WiFi kapcsolat bontva");
    }
}

String CardUID(byte *buffer, byte bufferSize) {
  String value = "";
  for (byte i = 0; i < bufferSize; i++) {
    value = value + buffer[i];
  }
  return value;
}
