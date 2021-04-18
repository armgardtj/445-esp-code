#include "ADXL335.h"

ADXL335 accelerometer;
void setup()
{
  Serial.begin(115200);
  accelerometer.begin();
}
void loop()
{
  float ax,ay,az;
  accelerometer.getAcceleration(&ax,&ay,&az);
  //Serial.println(az);
  delay(5);
}
