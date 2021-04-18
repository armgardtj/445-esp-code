#include <Adafruit_AHT10.h>
#include <Wire.h>

Adafruit_AHT10 aht;
TwoWire w = TwoWire(0);

void setup() {
  Serial.begin(115200);
  Serial.println("Adafruit AHT10 demo!");


  w.begin(9,8, 100000);
  while (!aht.begin(&w, 0)) {
    Serial.println("Could not find AHT10? Check wiring");
    delay(10);
  }
  Serial.println("AHT10 found");
}

void loop() {
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
  Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
  Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");

  delay(500);
}
