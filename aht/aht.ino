#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_AHT10.h>
#include <Wire.h>

Adafruit_AHT10 aht;
TwoWire w = TwoWire(0);

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
  w.begin(9,8, 100000);
  aht.begin(&w, 0);
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
  float h, t;
  measure_data(&h, &t);
  //determine_state(h);
  if (currState != lastState)
    send_http_request(currState); 
}

void measure_data(float *h, float *t)
{
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
//  Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
//  Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");
  *h = humidity.relative_humidity;
  *t = temp.temperature;
}

void determine_state(float v)
{
  if (lastState == true) { // appliance was on
    if (v >= onHighThreshold or v <= onLowThreshold) { // check if still on (breaks threshold)
      onCycles = maxOnCycles; // set timer to max value
      currState = true; // set appliance to on
    }
    else {
      if (onCycles-- <= 0){ // if device hasn't broke threshold in clock span
        currState = false; // set appliance to off
      }
    }
  } else { // appliance was off
    if (v >= onHighThreshold or v <= onLowThreshold) { // check if value broke threshold while off
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
  String payload = "{\"nodeID\": \"" + nodeID + "\", \"active\":" + (bool)deviceStatus + ", \"value\": " + String(deviceStatus) + "}";
  int httpResponseCode = http.PUT(payload);
  Serial.println(httpResponseCode);
  http.end();
}
