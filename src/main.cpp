#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#define address 99

char computerdata[20];
byte received_from_computer = 0;
byte serial_event = 0;
byte code = 0;
char ph_data[32];
byte in_char = 0;
byte i = 0;
int time_ = 815;
float ph_float;

#define WIFI_SSID "ssid"
#define WIFI_PASSWORD "password"
#define API_KEY "firebase api key"
#define DATABASE_URL "firebase database url"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

void setup()
{
  Serial.begin(115200);
  Wire.begin();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", ""))
  {
    Serial.println("ok");
    signupOK = true;
  }
  else
  {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void serialEvent()
{
  received_from_computer = Serial.readBytesUntil(13, computerdata, 20);
  computerdata[received_from_computer] = 0;
  serial_event = true;
}

void loop()
{
  computerdata[0] = 'r';
  for (int i = 1; i < 20; i++)
  {
    computerdata[i] = 0;
  }
  if (computerdata[0] == 'c' || computerdata[0] == 'r')
    time_ = 815;
  else
    time_ = 250;

  Wire.beginTransmission(address);
  Wire.write((uint8_t *)computerdata, 20);
  Wire.endTransmission();

  delay(time_);

  Wire.requestFrom(address, 32, 1);
  code = Wire.read();

  switch (code)
  {
  case 1:
    Serial.println("Success");
    break;

  case 2:
    Serial.println("Failed");
    break;

  case 254:
    Serial.println("Pending");
    break;

  case 255:
    Serial.println("No Data");
    break;
  }

  while (Wire.available())
  {
    in_char = Wire.read();
    ph_data[i] = in_char;
    i += 1;
    if (in_char == 0)
    {
      i = 0;
      break;
    }
  }

  Serial.println(ph_data);

  ph_float = atof(ph_data);

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();
    FirebaseJsonArray pHArray;
    if (Firebase.RTDB.getArray(&fbdo, "test/pH", &pHArray))
    {
      pHArray.add(ph_float);
      if (Firebase.RTDB.setArray(&fbdo, "test/pH", &pHArray))
      {
        Serial.println("PASSED");
        Serial.print("PATH: ");
        Serial.println(fbdo.dataPath());
        Serial.print("TYPE: ");
        Serial.println(fbdo.dataType());
      }
      else
      {
        Serial.println("FAILED");
        Serial.print("ERROR: ");
        Serial.println(fbdo.errorReason());
      }
    }
    else
    {
      Serial.println("FAILED TO RETRIEVE ARRAY");
      Serial.print("ERROR: ");
      Serial.println(fbdo.errorReason());
    }
  }
}
