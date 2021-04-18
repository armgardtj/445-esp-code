#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

RF24 radio(4,5,6,8,7);
int carriers = 0;
 
void setup(void)
{
  Serial.begin(115200);
  radio.begin();
  radio.setAutoAck(false);
  // Get into standby mode
  radio.startListening();
  radio.stopListening();
  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);
}
const int num_reps = 100;
const int channel = 50;
void loop(void)
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
  // Print out channel measurements, clamped to a single hex digit
  Serial.println(carriers);
  delay(5);
}
