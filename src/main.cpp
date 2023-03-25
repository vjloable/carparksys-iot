#include <Arduino.h>
#include <Firebase_ESP_Client.h>
#include <WiFi.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

#define LED_RED 15
#define LED_YLW 13
#define LED_GRN 12
#define USS_TRG 4
#define USS_ECH 2
#define API_KEY "AIzaSyC0HvBjCuM4Lu2SSoK03Yt2bkEkx4yjdkI"
#define DATABASE_URL "https://carparksys-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define USER_EMAIL "carpark.lot1a@gmail.com"
#define USER_PASSWORD "0000001A"

#define WIFI_SSID "Solace"
#define WIFI_PASS "littlelove"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseJson json;

unsigned long sendDataPrevMillis = 0;
bool toggle;
long duration;
int distance;
int count;
int state;
String uid;
String path;

void setup()
{
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.println("Connecting to WiFi...");
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

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YLW, OUTPUT);
  pinMode(LED_GRN, OUTPUT);
  pinMode(USS_TRG, OUTPUT);
  pinMode(USS_ECH, INPUT);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GRN, LOW);
  digitalWrite(LED_YLW, LOW);

  toggle = false;
  count = 5;
  state = 1;

  if (Firebase.RTDB.getInt(&fbdo, "/spaces/1A/status"))
  {
    if (fbdo.dataType() == "int")
    {
      if (fbdo.intData() == 1)
      {
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_GRN, HIGH);
        digitalWrite(LED_YLW, LOW);
        state = 1;
        toggle = false;
      }
      else if (fbdo.intData() == 2)
      {
        digitalWrite(LED_RED, HIGH);
        digitalWrite(LED_GRN, LOW);
        digitalWrite(LED_YLW, LOW);
        state = 2;
        toggle = true;
      }
      else if (fbdo.intData() == 3)
      {
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_GRN, LOW);
        digitalWrite(LED_YLW, HIGH);
        state = 3;
        toggle = false;
      }
    }
  }
  else
  {
    Serial.println();
  }
}

int readUltraSonic()
{
  delayMicroseconds(2);
  digitalWrite(USS_TRG, HIGH);
  delayMicroseconds(10);
  digitalWrite(USS_TRG, LOW);
  duration = pulseIn(USS_ECH, HIGH);
  Serial.print("Distance: ");
  Serial.println(distance);
  return (duration * 0.034 / 2);
}

void loop()
{
  count = 6;
  if(!toggle){
    if (Firebase.RTDB.getInt(&fbdo, "/spaces/1A/status"))
    {
      if (fbdo.dataType() == "int")
      {
        if (fbdo.intData() == 1)
        {
          digitalWrite(LED_RED, LOW);
          digitalWrite(LED_GRN, HIGH);
          digitalWrite(LED_YLW, LOW);
          state = 1;
          toggle = false;
        }
        else if (fbdo.intData() == 2)
        {
          digitalWrite(LED_RED, HIGH);
          digitalWrite(LED_GRN, LOW);
          digitalWrite(LED_YLW, LOW);
          state = 2;
          toggle = false;
          if (Firebase.ready() && millis() - sendDataPrevMillis > 100 || sendDataPrevMillis == 0)
          {
            sendDataPrevMillis = millis();
            if (Firebase.RTDB.setInt(&fbdo, "/spaces/1A/status", 2))
            {
              Serial.println("UPDATED");
            }
            else
            {
              Serial.println("ERROR");
            }
          }
        }
        else if (fbdo.intData() == 3)
        {
          digitalWrite(LED_RED, LOW);
          digitalWrite(LED_GRN, LOW);
          digitalWrite(LED_YLW, HIGH);
          state = 3;
          toggle = false;
        }
      }
    }
    else
    {
      Serial.println();
    }
  }

  if(!toggle){
    digitalWrite(USS_TRG, LOW);
    distance = readUltraSonic();
    if (state == 1 || state == 3)
    { // green or yellow
      if (distance <= 10)
      {
        toggle = true;
        Serial.println("TOGGLEEEEE!!");
      }
      else
      {
        toggle = false;
      }
    }
  }else{
    Serial.println("HEREEEEEEEEEEE!!");
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GRN, LOW);
    digitalWrite(LED_YLW, LOW);
    if (Firebase.ready() && millis() - sendDataPrevMillis > 100 || sendDataPrevMillis == 0)
    {
      sendDataPrevMillis = millis();
      if (Firebase.RTDB.setInt(&fbdo, "/spaces/1A/status", 2))
      {
        Serial.println("UPDATED");
      }
      else
      {
        Serial.println("ERROR");
      }
    }
    if (Firebase.RTDB.getString(&fbdo, "/spaces/1A/user"))
    {
      if (fbdo.dataType() == "string")
      {
        uid = String(fbdo.stringData());
      }
    }
    path = String("/users/");
    path += String(uid);
    path += String("/hasArrived/");
    Serial.println(path);
    if (Firebase.ready() && millis() - sendDataPrevMillis > 100 || sendDataPrevMillis == 0)
    {
      sendDataPrevMillis = millis();
      if (Firebase.RTDB.setBool(&fbdo, path, true))
      {
        Serial.println("UPDATED");
      }
      else
      {
        Serial.println("ERROR");
      }
    }
    if (Firebase.RTDB.getString(&fbdo, "/spaces/1A/user"))
    {
      if (fbdo.dataType() == "string")
      {
        uid = String(fbdo.stringData());
        Serial.println(uid);
      }
      else
      {
        Serial.println("CANT FIND UID");
      }
    }
    
    while(true){
      digitalWrite(USS_TRG, LOW);
      distance = readUltraSonic();
      if (distance <= 10)
      {
        count = 6;
      }
      else
      {
        count--;
      }
      delay(1000);

      Serial.println(count);

      if(count == 0){
        if (Firebase.RTDB.getString(&fbdo, "/spaces/1A/user"))
        {
          if (fbdo.dataType() == "string")
          {
            uid = String(fbdo.stringData());
            Serial.println(uid);
          }else{
            Serial.println("CANT FIND UID");
          }
        }
        path = String("/users/");
        path += String(uid);
        path += String("/has_ticket/");
        if (Firebase.ready() && millis() - sendDataPrevMillis > 100 || sendDataPrevMillis == 0)
        {
          sendDataPrevMillis = millis();
          if (Firebase.RTDB.setBool(&fbdo, path, false))
          {
            Serial.println("UPDATED");
          }
          else
          {
            Serial.println("ERROR");
          }
        }
        path = "/users/";
        path += String(uid);
        path += "/hasArrived/";
        Serial.println(path);
        if (Firebase.ready() && millis() - sendDataPrevMillis > 100 || sendDataPrevMillis == 0)
        {
          sendDataPrevMillis = millis();
          if (Firebase.RTDB.setInt(&fbdo, path, false))
          {
            Serial.println("UPDATED");
          }
          else
          {
            Serial.println("ERROR");
          }
        }

        if (Firebase.ready() && millis() - sendDataPrevMillis > 100 || sendDataPrevMillis == 0)
        {
          sendDataPrevMillis = millis();
          if (Firebase.RTDB.setInt(&fbdo, "/spaces/1A/status", 1))
          {
            Serial.println("UPDATED");
          }
          else
          {
            Serial.println("ERROR");
          }
        }
        if (Firebase.ready() && millis() - sendDataPrevMillis > 100 || sendDataPrevMillis == 0)
        {
          sendDataPrevMillis = millis();
          if (Firebase.RTDB.deleteNode(&fbdo, "/spaces/1A/user"))
          {
            Serial.println("UPDATED");
          }
          else
          {
            Serial.println("ERROR");
          }
        }
        toggle = false;
        break;
      }
    }
  }
}
