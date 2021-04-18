#include "ADXL335.h"

ADXL335 accelerometer;
bool isOff = true;
//String webServer = 
int powerButtonPin = 1;
bool powerButtonPressed = false;
int speaker = 2;
int led = 3;

void setup()
{
  Serial.begin(115200);

  pinMode(powerButtonPin, INPUT);
  pinMode(speaker, OUTPUT);
  pinMode(led, OUTPUT);
  
  accelerometer.begin();
}
void loop()
{
  Serial.println(digitalRead(powerButtonPin));
  //if (isOff)
  
  float ax,ay,az;
  accelerometer.getAcceleration(&ax,&ay,&az);
//  Serial.print(xvoltage); Serial.print(" ");
//  Serial.print(yvoltage); Serial.print(" ");
//  Serial.println(zvoltage);
  delay(5);
}
