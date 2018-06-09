#include <ES9028.h>

/*
  Initialises 2 Sabre32 ES9028/38 DACs in 24 bit dual mono operation using the Hybrid filter,
  sets input to SPDIF, initialises the attenuation to 25Db, sets the DPLL to 'Lowest',
  unmutes the DACs, and shows the lock status via the built in LED
*/
ES9028 leftDAC  = ES9028("Left Channel", ES9028::MonoLeft);
ES9028 rightDAC = ES9028("Right Channel", ES9028::MonoRight);

void configureDAC(ES9028 dac)
{
  dac.mute();
  dac.setSerialBits(ES9028::Bits_24);
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
}

void setup() {
  // initialize digital pin LED_BUILTIN as the DAC lock light.
  pinMode(LED_BUILTIN, OUTPUT);
  delay(1500);
  if (leftDAC.initialise())
    configureDAC(leftDAC);
  if (rightDAC.initialise())
    configureDAC(rightDAC);
  delay(500);
  leftDAC.unmute();
  rightDAC.unmute();
}

void loop() {
  digitalWrite(LED_BUILTIN, leftDAC.locked() && rightDAC.locked());   // turn the LED on if both DACs locked
  delay(100);
}
