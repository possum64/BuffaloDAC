#include <ES9018.h>

/*
  Initialises 2 Sabre32 ES9018 Buffalo DACs in dual mono operation with opposite phase odd/even channel outputs,
  sets input to SPDIF, initialises the attenuation to 25Db, sets the DPLL to 'Lowest',
  unmutes the DACs after 300 milliseconds, and shows the lock status via the built in LED
*/
ES9018 leftDAC  = ES9018(MonoLeft, InPhase, AntiPhase);
ES9018 rightDAC = ES9018(MonoRight, AntiPhase, InPhase);

void initDAC(ES9018 dac)
{
  dac.mute();
  dac.setInputMode(SPDIF);
  dac.setDPLL128Mode(UseDPLLSetting);
  dac.setDPLLMode(AllowAll);
  dac.setDPLL(Lowest);
  dac.setAttenuation(25);
  dac.init();  // init() writes all register values for the first time. After an init() any further register changes are written immediately
}

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  initDAC(leftDAC);
  initDAC(rightDAC);
  delay(300);
  leftDAC.unmute();
  rightDAC.unmute();
}

void loop() {
  digitalWrite(LED_BUILTIN, leftDAC.locked() && rightDAC.locked());   // turn the LED on if both DACs locked
}
