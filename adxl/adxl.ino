#include <WiFi.h>
#include "esp_wpa2.h"
#include <HTTPClient.h>

#include "ADXL335.h"

ADXL335 accelerometer;

//#define EAP_ANONYMOUS_IDENTITY "johnra2" //anonymous identity
#define EAP_IDENTITY "johnra2"                  //user identity
#define EAP_PASSWORD "Illinois6Killed^()64" //eduroam user password

const char* ssid = "RecycleNet";
const char* wifipass = "Birdemption69";
const String webServer = "http://33sd.ngrok.io/updateNode";
const String webGraph = "http://33sd.ngrok.io/updateGraph";
const String nodeID = "ib3vz222u7m0dib";

const int powerButtonPin = 1;
const int speaker = 2;
const int led = 3;

bool isOff = true;
bool powerButtonPressed = false;
bool buttonPressedLastCycle = false;
bool lastState = false;
bool currState = false;
int onCycles = 0;
int onCountsWhileOff = 0;
int buttonPressedCycles = 0;
int buttonPressedCount = 0;
const int maxButtonPressedCycles = 3000;
const int maxCalibrationCycles = 5000;
const int maxOnCycles = 1000;
float onThreshold = 121.0;

void setup()
{
  setup_board();
  setup_wifi(); 
  setup_sensor();
}

void setup_board()
{
  Serial.begin(115200);
  pinMode(powerButtonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(powerButtonPin), buttonInterrupt, RISING);
  pinMode(speaker, OUTPUT);
  pinMode(led, OUTPUT);
  digitalWrite(speaker, LOW);
  digitalWrite(led, LOW);
  buttonPressedCycles = 0;
  buttonPressedCount = 0;
}

void buttonInterrupt()
{
  buttonPressedCount++;
}

void setup_wifi()
{
  int b = 0;
  if (b){
    WiFi.mode(WIFI_STA); //init wifi mode
    //esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_ANONYMOUS_IDENTITY, strlen(EAP_ANONYMOUS_IDENTITY)); //provide identity
    esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY)); //provide username
    esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD)); //provide password
    esp_wifi_sta_wpa2_ent_enable();
    WiFi.begin(ssid); //connect to wifi
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
  accelerometer.begin();
}

void calibrate()
{
  buttonPressedCount = 0;
  Serial.println("Calibration mode");
  Serial.println("Press button to measure off state");
  while (buttonPressedCount < 1)
    delay(5);
  Serial.println("3");
  delay(1000);
  Serial.println("2");
  delay(1000);
  Serial.println("1");
  delay(1000);
  int calibrationCycles = maxCalibrationCycles;
  float offVal = 0.0;
  while (calibrationCycles-- > 0) {
    float x,y,z;
    measure_data(&x,&y,&z);
    offVal += z;
    //Serial.print(z); Serial.print(" "); Serial.print(offVal); Serial.print(" "); Serial.println(calibrationCycles);
  }
  offVal /= maxCalibrationCycles;
  buttonPressedCount = 0;
  Serial.println("Press button to measure on state");
  while (buttonPressedCount < 1)
    delay(5);
  Serial.println("3");
  delay(1000);
  Serial.println("2");
  delay(1000);
  Serial.println("1");
  delay(1000);
  calibrationCycles = maxCalibrationCycles;
  float onVal = 0.0;
  while (calibrationCycles-- > 0) {
    float x,y,z;
    measure_data(&x,&y,&z);
    onVal += abs(z - offVal);
    //Serial.print(z); Serial.print(" "); Serial.print(onVal); Serial.print(" "); Serial.println(calibrationCycles);
  }
  onVal /= maxCalibrationCycles;
  onThreshold = onVal + offVal;
  Serial.println(onThreshold);
}

void loop()
{
  powerButtonPressed = digitalRead(powerButtonPin);
  //Serial.print(buttonPressedCount); Serial.print(" "); Serial.print(buttonPressedCycles); Serial.print(" "); Serial.println(isOff);
  if (isOff) {
    off_handler();
  } else {
    on_handler();
  }
  buttonPressedLastCycle = powerButtonPressed;
  lastState = currState;
}

void off_handler()
{
//  Serial.println("off");
  digitalWrite(led, LOW);
  if (buttonPressedCount > 0) {
    isOff = false;
    digitalWrite(led, HIGH);
    buttonPressedCount = 0;
  }
}

void on_handler()
{
//  Serial.println("on");
  
  if (evaluate_button()){
    buttonPressedCount = 0;
    return;
  }
  float x,y,z;
  measure_data(&x,&y,&z);
  determine_state(z);
  //send_graph_request(z);
  if (currState != lastState)
    send_http_request(currState); 
//  Serial.print(currState); Serial.print(" "); Serial.print(lastState); Serial.print(" "); Serial.print(z); Serial.print(" "); Serial.println(onThreshold);
}

boolean evaluate_button()
{
  if (powerButtonPressed){
//    Serial.println("bruh");
    buttonPressedCycles++;
    if (buttonPressedCycles >= maxButtonPressedCycles){
      isOff = true;
      buttonPressedCycles = 0;
      return true;
    }
  } else {
    buttonPressedCycles = 0;
  }
  if (buttonPressedCount >= 3){
    calibrate();
    return true;
  } 
  return false;
}

void measure_data(float *x, float *y, float *z)
{
  accelerometer.getAcceleration(x,y,z);
//  Serial.print(x); Serial.print(" ");
//  Serial.print(y); Serial.print(" ");
//  Serial.println(*z);
}

void determine_state(float v)
{
  if (lastState == true) { // appliance was on
    if (v >= onThreshold) { // check if still on (breaks threshold)
      onCycles = maxOnCycles; // set timer to max value
      currState = true; // set appliance to on
    }
    else {
      if (onCycles-- <= 0){ // if device hasn't broke threshold in clock span
        currState = false; // set appliance to off
      }
    }
  } else { // appliance was off
    if (v >= onThreshold) { // check if value broke threshold while off
      if (onCountsWhileOff++ == 0) { // if first time, start counting over number of cycles
        onCycles = maxOnCycles; // reset cycle counter
      }
      else if (onCountsWhileOff == 10) { // if number if hits exceeds 10
        currState = true; // mark appliance as on

      }
    }
    else {
      if (onCycles-- <= 0){ // decrement number of cycles
        onCountsWhileOff = 0; // set hits to 0 if cycle finishes
      }
    }
  }
}

void send_http_request(int deviceStatus) 
{
  //Serial.println(deviceStatus);
  Serial.print("Sending HTTP request for ");
  if (deviceStatus)
    Serial.println("on state");
  else
    Serial.println("off state");
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("disconnected");
    return;
  }
  HTTPClient http;
  http.begin(webServer.c_str());
  http.addHeader("Content-Type", "application/json");
  String payload = "{\"nodeID\": \"" + nodeID + "\", \"active\":" + (bool)deviceStatus + ", \"value\": " + String(deviceStatus) + "}";
  int httpResponseCode = http.PUT(payload);
//  Serial.println(httpResponseCode);
  http.end();
  Serial.println("Response received");
}

void send_graph_request(float deviceStatus) 
{
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("disconnected");
    return;
  }
  HTTPClient http;
  http.begin(webGraph.c_str());
  http.addHeader("Content-Type", "application/json");
  String payload = "{\"nodeID\": \"" + nodeID + "\", \"value\": " + String(deviceStatus) + "}";
  int httpResponseCode = http.PUT(payload);
//  Serial.println(httpResponseCode);
  http.end();
}
