#include <ES9018.h>

/*
  Initialises 2 Sabre32 ES9018 Buffalo DACs in dual mono operation with opposite phase outputs,
  initialises the attenuation to 25Db, sets the DPLL to 'Lowest'
  and shows the lock status via the builting LED
*/
ES9018 leftDAC  = ES9018(MonoLeft, InPhase, AntiPhase);
ES9018 rightDAC = ES9018(MonoRight, AntiPhase, InPhase);

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  leftDAC.mute();
  rightDAC.mute();
  leftDAC.init();
  rightDAC.init();
  leftDAC.setDPLL128Mode(UseDPLLSetting);
  rightDAC.setDPLL128Mode(UseDPLLSetting);
  leftDAC.setDPLLMode(AllowAll);
  rightDAC.setDPLLMode(AllowAll);
  leftDAC.setDPLL(Lowest);
  rightDAC.setDPLL(Lowest);
  leftDAC.setAttenuation(25);
  rightDAC.setAttenuation(25);
  delay(300);
  leftDAC.unmute();
  rightDAC.unmute();
}

void loop() {
  digitalWrite(LED_BUILTIN, leftDAC.locked() && rightDAC.locked());   // turn the LED on if both DACs locked
}
