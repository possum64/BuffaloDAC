#include <ES9018.h>

/*
  Initialises a Sabre32 ES9018 Buffalo DAC in stereo operation (default input is I2S or DSD),
  initialises the attenuation to 25dB, sets the DPLL to 'Lowest'
  unmutes the DACs after 300 milliseconds, and shows the lock status via the built in LED
*/
ES9018 dac = ES9018(Stereo);

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  dac.mute();
  dac.setDPLL128Mode(UseDPLLSetting);
  dac.setDPLLMode(AllowAll);
  dac.setDPLL(Lowest);
  dac.setAttenuation(25);
  dac.init();  // init() writes all register values for the first time. After an init() any further register changes are written immediately
  delay(300);
  dac.unmute();
}

void loop() {
  digitalWrite(LED_BUILTIN, dac.locked());   // turn the LED on if locked
}
