#include <ES9028.h>

/*
  Initialises a Sabre32 ES9028/38 DAC in 24 bit to operate in quadraphonic using the Hybrid filter,
  where input channel 1 provides the front left right and input channel 2 provides surround.
  (Note: This setup could also be used in a 3-way xover scenario)
  Initialises the attenuation to 25Db, sets the DPLL to 'Low',
  unmutes the DACs, and shows the lock status via the built in LED
*/

// NOTE: mapInputs must be called to reconfigure from defult 8-channel input mode 
ES9028 dac  = ES9028("Quadrophonic DAC"); 

void configureDAC(ES9028 dac)
{
}

void setup() {
  // initialize digital pin LED_BUILTIN as the DAC lock light.
  pinMode(LED_BUILTIN, OUTPUT);
  delay(1500);
  if (dac.initialise())
  {
    dac.mute();
    dac.setSerialBits(ES9028::Bits_24);
    // map DAC channels 1-4 to inputs 1 & 2 (first I2S data input), and DAC channels 5-8 to inputs 3 & 4 (second I2S data input)
    dac.mapInputs(ES9028::Input_1, ES9028::Input_2, ES9028::Input_1, ES9028::Input_2, ES9028::Input_3, ES9028::Input_4, ES9028::Input_3, ES9028::Input_4);
    dac.setInputSelect(ES9028::InputSelect_SPDIF);
    dac.setAutoSelect(ES9028::AutoSelect_Disable);
    dac.setFilterShape(ES9028::Filter_Hybrid);
    dac.setAutoMute(ES9028::AutoMute_MuteAndRampToGnd);
    dac.setAutomuteTime(100);
    dac.setGPIO1(ES9028::GPIO_Automute);
    dac.setGPIO2(ES9028::GPIO_StandardInput);
    dac.setGPIO3(ES9028::GPIO_StandardInput);
    dac.setGPIO4(ES9028::GPIO_Lock);
    dac.setDpllBandwidthSerial(ES9028::DPLL_Lowest);
    dac.setVolumeMode(ES9028::Volume_UseChannel1);
    dac.setAttenuation(25);
    delay(500);
    dac.unmute();
  }
}

void loop() {
  digitalWrite(LED_BUILTIN, dac.locked());   // turn the LED on if both DACs locked
  delay(100);
}
