#ifndef DACVolumeControl_h
#define DACVolumeControl_h
#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
  #include "pins_arduino.h"
  #include "WConstants.h"
#endif
#include <DACControl.h>

class DACVolumeControl
{
  public:

    DACVolumeControl(DACControl* dacCtrl, byte pinAnalogInput);
    DACVolumeControl(DACControl* dacCtrl, byte pinAnalogInput, byte pinMotor1, byte pinMotor2);
    void loop();
    void volumeUp();
    void volumeDown();
    void initialise();

  private: 

    DACControl* _dacCtrl;
    byte _pinMotor1 = 255;
    byte _pinMotor2 = 255;
    byte _pinAnalogInput;
    int _pot = 0;
    unsigned int _delayPot = 0;                          // delay before turning potentiometer motor off
    unsigned long _lastPotEvent = 0;                     // last time volume pot changed

    void _setAttenuationFromPot();
    bool _motorised();
};

#endif
