#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

RF24 radio(4,5,6,8,7);
int carriers = 0;

const char* ssid = "RecycleNet";
const char* wifipass = "Birdemption69";
const String webServer = "http://33sd.ngrok.io/updateNode";
const String nodeID = "bspv7m0b86mv64j";

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
const float onHighThreshold = 6;
const float onLowThreshold = 0;
const int num_reps = 100;
const int channel = 50;
 
void setup()
{
  setup_board();
//  setup_wifi();
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
  radio.begin();
  radio.setAutoAck(false);
  // Get into standby mode
  radio.startListening();
  radio.stopListening();
  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);
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
  int c;
  measure_data(&c);
  Serial.println(c);
  determine_state(c);
//  if (currState != lastState)
//    send_http_request(currState); 
}

void measure_data(*c)
{
  carriers = 0;
  // Scan all channels num_reps times
  int rep_counter = num_reps;
  while (rep_counter--)
  {
    radio.setChannel(channel);
    // Listen for a little
    radio.startListening();
    delayMicroseconds(225);
    if (radio.testCarrier()){
      ++carriers;
    }
    radio.stopListening();
  }
  *c = carriers;
}

void determine_state(int v)
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

void send_http_request(int deviceStatus) 
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
