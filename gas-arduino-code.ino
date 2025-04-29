#include <Wire.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <Servo.h>
#include <LiquidCrystal_I2C.h>

#define mainBulb D3
#define reservedBulb D4
#define gas A0

LiquidCrystal_I2C lcd(0x3F, 16, 2);

#define WIFI_SSID "G-Coder"
#define WIFI_PASSWORD "G-Coder@02"

#define API_KEY "AIzaSyBNZ1aVpWALpintHstXLPC9uyC992zzRfw"
#define DATABASE_URL "gas-monitoring-a2b73-default-rtdb.firebaseio.com/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;

int data = 0;
int sensorThres = 100;


unsigned long sendDataPrevMillis = 0;

void setup() {
  lcd.begin(16, 2);
  lcd.init();
  lcd.clear();         
  lcd.backlight();
  pinMode(mainBulb, OUTPUT);
  pinMode(reservedBulb, OUTPUT);
  digitalWrite(mainBulb, HIGH);
  digitalWrite(reservedBulb, HIGH);
  Serial.begin(9600);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase connection successful");
    signupOK = true;
  } else {
    Serial.printf("Firebase sign-up error: %s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void gasValue(){
  int data = analogRead(gas);
  Serial.print("Gas: ");
  Serial.println(data);
  lcd.setCursor(2,0);   
  lcd.print("Gas: ");
  lcd.println(data);
  Firebase.RTDB.setInt(&fbdo, "monitor/gas", data);
  if (data > 999) {
    Serial.println("Alert gas leaked!");
    lcd.clear();
    lcd.setCursor(5,0);   
    lcd.print("Alert!");
    lcd.setCursor(2,0);   
    lcd.print("Gas leaked");
  }
}


void loop() {
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 500 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    gasValue();
    if (Firebase.RTDB.getBool(&fbdo, "controls/mainBulb")) {
      if (fbdo.dataType() == "boolean"){
      bool mainBulbStateStr = fbdo.boolData();
      Serial.println("Seccess: " + fbdo.dataPath() + ": " + mainBulbStateStr + "(" + fbdo.dataType() + ")");
      bool mainBulbState = (mainBulbStateStr == false) ? HIGH : LOW;
      digitalWrite(mainBulb, mainBulbState);   
      }
    } else {
      Serial.println("Failed to read Auto: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.getBool(&fbdo, "controls/reservedBulb")) {
      if (fbdo.dataType() == "boolean"){
      bool reservedBulbStateStr = fbdo.boolData();
      Serial.println("Seccess: " + fbdo.dataPath() + ": " + reservedBulbStateStr + "(" + fbdo.dataType() + ")");
      bool reservedBulbState = (reservedBulbStateStr == false) ? HIGH : LOW;
      digitalWrite(reservedBulb, reservedBulbState);   
      }
    } else {
      Serial.println("Failed to read Auto: " + fbdo.errorReason());
    }

  

    Serial.println("_______________________________________");
  }
}




