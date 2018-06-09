#include <ES9018.h>

/*
  Initialises a Sabre32 ES9018 Buffalo DAC in stereo operation (default input is I2S or DSD),
  initialises the attenuation to 25dB, sets the DPLL to 'Lowest'
  unmutes the DACs , and shows the lock status via the built in LED
*/
ES9018 dac = ES9018("Stereo DAC", ES9018::Clock100Mhz);

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  delay(1500);
  if (dac.initialise())
  {
    dac.mute();
    dac.setDPLL128Mode(ES9018::UseDPLLSetting);
    dac.setDPLLMode(ES9018::AllowAll);
    dac.setDPLL(ES9018::Lowest);
    dac.setAttenuation(25);
    delay(300);
    dac.unmute();
  }
}

void loop() {
  digitalWrite(LED_BUILTIN, dac.locked());   // turn the LED on if locked
  delay(100);
}
