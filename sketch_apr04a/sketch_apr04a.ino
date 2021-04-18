
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
//
// Hardware configuration
//
 
// Set up nRF24L01 radio on SPI bus plus CE and CSN
// CE  - Chip Enable
// CSN - Chip Select
 
#define CE_PIN  9
#define CSN_PIN 10
 
RF24 radio(CE_PIN,CSN_PIN);
 
//
// Channel info
//
 
const uint8_t num_channels = 128;
uint8_t values[num_channels];
 
//
// Setup
//
 
void setup(void)
{
  //
  // Print preamble
  //
 
  Serial.begin(115200);
  printf_begin();
  printf("\n\rRF24/examples/scanner/\n\r");
 
  //
  // Setup and configure rf radio
  //
 
  radio.begin();
  radio.setAutoAck(false);
 
  // Get into standby mode
  radio.startListening();
  radio.stopListening();
 
  // Print out header, high then low digit
  int i = 0;
  while ( i < num_channels )
  {
    printf("%x",i>>4);
    ++i;
  }
  printf("\n\r");
  i = 0;
  while ( i < num_channels )
  {
    printf("%x",i&0xf);
    ++i;
  }
  printf("\n\r");
}
 
//
// Loop
//
 
const int num_reps = 100;
 
void loop(void)
{
  // Clear measurement values
  memset(values,0,sizeof(values));
 
  // Scan all channels num_reps times
  int rep_counter = num_reps;
  while (rep_counter--)
  {
    int i = 50;
    // Select this channel
    radio.setChannel(i);

    // Listen for a little
    radio.startListening();
    delayMicroseconds(225);
    

    // Did we get a carrier?
    if ( radio.testCarrier() ){
      ++values[i];
    }
    radio.stopListening();
  }
 
  // Print out channel measurements, clamped to a single hex digit
  int i = 50;
  Serial.println(values[i]);

  delay(5);
  //printf("\n\r");
}
 
