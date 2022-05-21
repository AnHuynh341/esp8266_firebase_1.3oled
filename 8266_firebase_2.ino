#include <Arduino.h>

#include <Wire.h>

#include "SH1106Wire.h" //alis for `#include "SH1106Wire.h"`

#include "Graphic_esp8266_dht22_oledi2c.h"

SH1106Wire display(0x3c, D1, D2);


#include "DHT.h"
#define DHTpin 2 //D4
#define DHTTYPE DHT11
DHT dht(DHTpin,DHTTYPE);


#include "NTPClient.h"
#include "ESP8266WiFi.h"
#include "WiFiUdp.h"


#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "Wifi name"
#define WIFI_PASSWORD "password"

#define USER_EMAIL "your firebase email"
#define USER_PASSWORD "password"

// Insert Firebase project API Key
#define API_KEY "API_KEY"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "URL" 

FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int led1;
int led2;

int humthres;
int tempthres;
bool signupOK = false;

int count = 0;

#define RL1 D5
#define RL2 D6
#define RL3 D7
#define RL4 D8


void setup() {
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

  /* Assign the api key (required) */
  config.api_key = API_KEY;

    // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  }
  else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

   display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);

  dht.begin(); // initialize dht
  esp_begin();
  
  
    pinMode(RL1, OUTPUT);
    pinMode(RL2, OUTPUT);
    pinMode(RL3, OUTPUT);
    pinMode(RL4, OUTPUT);
 
    delay(1000);
}

void loop() {
  dht_sensor_getdata();
 displayTempHumid();
  readData();
 threshold();
 delay(2000);
 displayTempHumid();
   readData();
 threshold();
 delay(2000);
 displayTempHumid();
   readData();
 threshold();
 delay(1000);
}

void esp_begin()
{
  display.clear();
  display.drawXbm(0, 15, 128, 35, img_esp8266);
  display.display();
  delay(2000);
}


void readData() {
    if (Firebase.RTDB.getInt(&fbdo, "/LivingRoom/Led_1")) {
      if (fbdo.dataType() == "int") {
        led1 = fbdo.intData();
         Serial.print("Relay 1:" );
        Serial.println(led1);
      }
    }
      else {
       Serial.println(fbdo.errorReason());
       }

       if (led1 == 0) {
          digitalWrite(RL1, HIGH);
        }
        else {
          digitalWrite(RL1, LOW);
         }
    
  if (Firebase.RTDB.getInt(&fbdo, "/LivingRoom/Led_2")) {
      if (fbdo.dataType() == "int") {
        led2 = fbdo.intData();
        Serial.print("Relay 2:" );
        Serial.println(led2);
      }
    }
    else {
      Serial.println(fbdo.errorReason());
    }
           if (led2 == 0) {
          digitalWrite(RL2, HIGH);
        }
        else {
          digitalWrite(RL2, LOW);
         }
    delay(1000);
}

void dht_sensor_getdata()
  {
    float hm=dht.readHumidity();
    Firebase.RTDB.setFloat(&fbdo, "LivingRoom/Humidity",dht.readHumidity());
    Serial.print("Humidity ");
    Serial.println(hm);
    float temp=dht.readTemperature();
    Firebase.RTDB.setFloat(&fbdo, "LivingRoom/Temperature",dht.readTemperature());
     Serial.print("Temperature ");
    Serial.println(temp);
    float humidity=hm;
    float temperature=temp;
    delay(1000);
  }


void threshold()
{
  float tempthres;
  Firebase.RTDB.getFloat(&fbdo, "/LivingRoom/TempThres");
  tempthres=fbdo.floatData();
  Serial.print("Temp thres: ");
  Serial.println(tempthres);
  if (tempthres < dht.readTemperature())
  {
    digitalWrite(RL3, LOW);
  }
  else {
    digitalWrite(RL3, HIGH);
  }
 
  float humthres;
  Firebase.RTDB.getFloat(&fbdo, "/LivingRoom/HumThres");
  humthres=fbdo.floatData();
  Serial.print("Hum Thres: " );
  Serial.println(humthres);
    if (humthres > dht.readHumidity())
  {
    digitalWrite(RL4, LOW);
  }
  else {
    digitalWrite(RL4, HIGH);
  }
  delay(1000);
}

void displayTempHumid() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t) ) {   // || isnan(f)
    display.clear(); 
    display.setFont(ArialMT_Plain_16);
    display.drawString(5, 0, "Connect DHT11");
    display.display();
    delay(2000);
    return;
    }
  display.setColor(WHITE);
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.drawString(10, 0, "TEMPERATURE");
  display.setFont(ArialMT_Plain_16);
  display.drawString(10, 13, String(t) + " C");
  display.setFont(ArialMT_Plain_10);
  display.drawString(10, 32, "HUMIDITY");
  display.setFont(ArialMT_Plain_16);
  display.drawString(10, 45, String(h) + " %");
  display.display();
  delay(1000);
}



  

  
