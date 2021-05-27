#include <WiFi.h>
#include "esp_wpa2.h"
#include <HTTPClient.h>
#include <Adafruit_AHT10.h>
#include <Wire.h>

Adafruit_AHT10 aht;
TwoWire w = TwoWire(0);

//#define EAP_ANONYMOUS_IDENTITY "johnra2" //anonymous identity
#define EAP_IDENTITY "johnra2"                  //user identity
#define EAP_PASSWORD "Illinois6Killed^()64" //eduroam user password

const char* ssid = "RecycleNet";
const char* enterprisessid = "IllinoisNet";
const char* wifipass = "Birdemption69";
const String webServer = "http://33sd.ngrok.io/updateNode";
const String webGraph = "http://33sd.ngrok.io/updateGraph";
const String nodeID = "gqeqhver74ke1c6";

const int powerButtonPin = 1;
const int speaker = 2;
const int led = 3;

bool isOff = true;
bool powerButtonPressed = false;
bool buttonPressedLastCycle = false;
int onCycles = 0;
int onCountsWhileOff = 0;
const int maxOnCycles = 2000;
const float onHighThreshold = 121.0;
const float onLowThreshold = 110.0;

void setup() 
{
  setup_board();
  setup_wifi(); 
  setup_sensor();
}

void setup_board()
{
  Serial.begin(115200);
  pinMode(powerButtonPin, INPUT);
  pinMode(speaker, OUTPUT);
  pinMode(led, OUTPUT);
  digitalWrite(speaker, LOW);
  digitalWrite(led, LOW);
}

void setup_wifi()
{
  int b = 1;
  if (b){
    WiFi.mode(WIFI_STA); //init wifi mode
    //esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_ANONYMOUS_IDENTITY, strlen(EAP_ANONYMOUS_IDENTITY)); //provide identity
    esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY)); //provide username
    esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD)); //provide password
    esp_wifi_sta_wpa2_ent_enable();
    WiFi.begin(enterprisessid); //connect to wifi
  } else {
    WiFi.begin(ssid, wifipass);
  }
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void setup_sensor()
{
  w.begin(9,8, 100000);
  while (!aht.begin(&w, 0)) {
    Serial.println("Could not find AHT10? Check wiring");
    delay(10);
  }
}

void loop()
{
  powerButtonPressed = digitalRead(powerButtonPin);
  //Serial.println(powerButtonPressed);
//  if (isOff) {
//    off_handler();
//  }
//  else {
//    on_handler();
//  }
  on_handler();
  buttonPressedLastCycle = powerButtonPressed;
}

void off_handler()
{
  if (powerButtonPressed) {
    isOff = false;
    digitalWrite(led, HIGH);
  }
}

void on_handler()
{
  float h, t;
  measure_data(&h, &t);
  send_http_request(h); 
  delay(500);
}

void measure_data(float *h, float *t)
{
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
//  Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
  Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");
  *h = humidity.relative_humidity;
  *t = temp.temperature;
}

void send_http_request(float deviceStatus) 
{
  Serial.println(deviceStatus);
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("disconnected");
    return;
  }
  HTTPClient http;
  http.begin(webServer.c_str());
  http.addHeader("Content-Type", "application/json");
  String payload = "{\"nodeID\": \"" + nodeID + "\", \"active\":" + (bool)deviceStatus + "}";
  int httpResponseCode = http.PUT(payload);
  Serial.println(httpResponseCode);
  http.end();
  delay(100);
  http.begin(webGraph.c_str());
  http.addHeader("Content-Type", "application/json");
  payload = "{\"nodeID\": \"" + nodeID + "\", \"value\": " + String(deviceStatus) + "}";
  httpResponseCode = http.PUT(payload);
  Serial.println(httpResponseCode);
  http.end();
}
