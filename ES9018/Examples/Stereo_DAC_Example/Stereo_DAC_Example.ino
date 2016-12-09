#include <ES9018.h>

/*
  Initialises a Sabre32 ES9018 Buffalo DAC in stereo operation,
  initialises the attenuation to 25Db, sets the DPLL to 'Lowest'
  and shows the lock status via the builting LED
*/
ES9018 dac = ES9018(Stereo);

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  dac.mute();
  dac.init();
  dac.setDPLL128Mode(UseDPLLSetting);
  dac.setDPLLMode(AllowAll);
  dac.setDPLL(Lowest);
  dac.setAttenuation(25);
  delay(300);
  dac.unmute();
}

void loop() {
  digitalWrite(LED_BUILTIN, dac.locked());   // turn the LED on if locked
}
