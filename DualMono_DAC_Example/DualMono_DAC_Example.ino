#include <ES9028.h>
#include <DACControl.h>
#include <DACVolumeControl.h>

/*
  Initialises 2 Sabre32 ES9028/38 DACs in 24 bit dual mono operation using the Hybrid filter,
  sets input to SPDIF, initialises the attenuation to 25Db, sets the DPLL to 'Lowest',
  unmutes the DACs, and shows the lock status via the built in LED
*/

// Arduino analog pins
const byte VOL_ANALOG_INPUT_PIN = 0;

// Arduino digital pins
const byte POWER_RELAY = 3;
const byte DAC_RESET = 13;

#define NO_OF_ES9028_DACS 2
ES9028 leftDAC  = ES9028("Left Channel", ES9028::MonoLeft);
ES9028 rightDAC = ES9028("Right Channel", ES9028::MonoRight);
ES9028 es9028dacs[NO_OF_ES9028_DACS] = {leftDAC, rightDAC};
DACControl dacCtrl = DACControl(es9028dacs, NO_OF_ES9028_DACS, 40000);
DACVolumeControl dacVolCtrl = DACVolumeControl(&dacCtrl, VOL_ANALOG_INPUT_PIN);
// specify pins for motorised pot
//DACVolumeControl dacVolCtrl = DACVolumeControl(&dacCtrl, VOL_ANALOG_INPUT_PIN);

bool configDAC(ES9028* dac)
{
  dac->mute();
  dac->setSerialBits(ES9028::Bits_24);
  dac->setInputSelect(ES9028::InputSelect_SPDIF);
  dac->setAutoSelect(ES9028::AutoSelect_Disable);
  dac->setFilterShape(ES9028::Filter_Hybrid);
  dac->setAutoMute(ES9028::AutoMute_MuteAndRampToGnd);
  dac->setAutomuteTime(100);
  dac->setGPIO1(ES9028::GPIO_Automute);
  dac->setGPIO2(ES9028::GPIO_StandardInput);
  dac->setGPIO3(ES9028::GPIO_StandardInput);
  dac->setGPIO4(ES9028::GPIO_Lock);
  dac->setDpllBandwidthSerial(ES9028::DPLL_Lowest);
  dac->setVolumeMode(ES9028::Volume_UseChannel1);
  dac->setAttenuation(25);
  return true;
}

void eventLocked()
{
  digitalWrite(LED_BUILTIN, true);   // turn the LED on if both DACs locked
}

void eventNoLock()
{
  digitalWrite(LED_BUILTIN, false);   // turn the LED off if either DAC not locked
}

void setup() {
  // initialize digital pin LED_BUILTIN as the DAC lock light.
  pinMode(LED_BUILTIN, OUTPUT);
  dacCtrl.initES9028(configDAC);
  dacCtrl.setPinPowerRelay(POWER_RELAY);
  dacCtrl.setPinDACReset(DAC_RESET);
  dacCtrl.onLock(eventLocked);
  dacCtrl.onNoLock(eventNoLock);

  // customise I2C pins when using ESP8266
  // dacCtrl.setPinSDA(4);
  // dacCtrl.setPinSCL(5);

  // some other events that can be intercepted
  //   dacCtrl.onNotInitialised(eventDacInitialiseFailed);
  //   dacCtrl.onInitialised(eventDacInitialised);
  //   dacCtrl.onBeforePowerOff(eventBeforePowerOff);
  //   dacCtrl.onAfterPowerOff(eventAfterPowerOff);
  //   dacCtrl.onBeforePowerOn(eventBeforePowerOn);
  //   dacCtrl.onAfterPowerOn(eventAfterPowerOn);
  //   dacCtrl.onLockReadError(eventLockReadError);
  //   dacCtrl.onAutomuteStatusChanged(eventAutomuteStatusChanged);
  dacCtrl.powerOn();
}

void loop() {
  dacCtrl.loop();
  dacVolCtrl.loop();
  delay(10);
}
