#include <WiFi.h>
#include <HTTPClient.h>

#include "ADXL335.h"

ADXL335 accelerometer;

const char* ssid = "RecycleNet";
const char* wifipass = "Birdemption69";
const String webServer = "http://33sd.ngrok.io/updateNode";
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
const int maxCalibrationCycles = 5000;
const int maxOnCycles = 2000;
float onThreshold = 121.0;

void setup()
{
  setup_board();
  setup_wifi(); 
  setup_sensor();
  calibrate();
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
  WiFi.begin(ssid, wifipass);
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
  Serial.println("Calibration mode");
  Serial.println("Recording off data in");
  delay(1000);
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
  Serial.println("Recording on data in");
  delay(1000);
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
  //Serial.println(powerButtonPressed);
//  if (isOff) {
//    off_handler();
//  }
//  else {
//    on_handler();
//  }
  on_handler();
  buttonPressedLastCycle = powerButtonPressed;
  lastState = currState;
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
  float x,y,z;
  measure_data(&x,&y,&z);
  determine_state(z);
  if (currState != lastState)
    send_http_request(currState); 
}

void measure_data(float *x, float *y, float *z)
{
  accelerometer.getAcceleration(x,y,z);
//  Serial.print(x); Serial.print(" ");
//  Serial.print(y); Serial.print(" ");
//  Serial.println(z);
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
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("disconnected");
    return;
  }
  HTTPClient http;
  http.begin(webServer.c_str());
  http.addHeader("Content-Type", "application/json");
  String payload = "{\"nodeID\": \"" + nodeID + "\", \"active\":" + (bool)deviceStatus + ", \"value\": " + String(deviceStatus) + "}";
  int httpResponseCode = http.PUT(payload);
  //Serial.println(httpResponseCode);
  http.end();
}
