#include <ES9018.h>

/*
  Initialises 2 Sabre32 ES9018 Buffalo DACs in dual mono operation with opposite phase odd/even channel outputs,
  sets input to SPDIF, initialises the attenuation to 25Db, sets the DPLL to 'Lowest',
  unmutes the DACs after 300 milliseconds, and shows the lock status via the built in LED
*/
ES9018 leftDAC  = ES9018("Left Channel", ES9018::Clock100Mhz, ES9018::MonoLeft, ES9018::InPhase, ES9018::AntiPhase);
ES9018 rightDAC = ES9018("Right Channel", ES9018::Clock100Mhz, ES9018::MonoRight, ES9018::AntiPhase, ES9018::InPhase);

void configureDAC(ES9018 dac)
{
  dac.mute();
  dac.setInputSelect(ES9018::SPDIF);
  dac.setDPLL128Mode(ES9018::UseDPLLSetting);
  dac.setDPLLMode(ES9018::AllowAll);
  dac.setDPLL(ES9018::Lowest);
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
