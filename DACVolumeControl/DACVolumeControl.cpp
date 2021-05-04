#include <DACVolumeControl.h>

  DACVolumeControl::DACVolumeControl(DACControl* dacCtrl, byte pinAnalogInput)
  {
    _dacCtrl = dacCtrl;
    _pinAnalogInput = pinAnalogInput;
  };
  
  DACVolumeControl::DACVolumeControl(DACControl* dacCtrl, byte pinAnalogInput, byte pinMotor1, byte pinMotor2)
  {
    _dacCtrl = dacCtrl;
    _pinAnalogInput = pinAnalogInput;
    _pinMotor1 = pinMotor1;
    _pinMotor2 = pinMotor2;
    pinMode(_pinMotor1, OUTPUT);
    pinMode(_pinMotor2, OUTPUT);
  };
  
  void DACVolumeControl::loop()
  {
    if (_dacCtrl->getPower() && _dacCtrl->initialised())
    {
      //read volume pot
      if (_delayPot != 0)
        if (millis() - _lastPotEvent >= _delayPot)
        {
          if (_motorised())
          {
            // turn off volume pot
            digitalWrite(_pinMotor1, LOW);
            digitalWrite(_pinMotor2, LOW);
            _delayPot = 0;
            Serial.println(F("potentiometer stopped"));
          }
        }
      _setAttenuationFromPot();
    }
  };

  void DACVolumeControl::volumeUp()
  {
    if (_motorised())
      if (_dacCtrl->getPower())
      {
        digitalWrite(_pinMotor1, LOW);
        digitalWrite(_pinMotor2, HIGH);
        _delayPot = 120;
        _lastPotEvent = millis();
      }
  };
  
  void DACVolumeControl::volumeDown()
  {
    if (_motorised())
      if (_dacCtrl->getPower())
      {
        digitalWrite(_pinMotor1, HIGH);
        digitalWrite(_pinMotor2, LOW);
        _delayPot = 120;
        _lastPotEvent = millis();
      }
  };

  void DACVolumeControl::initialise()
  {
   _pot = 0;
   _setAttenuationFromPot();
  };

  void DACVolumeControl::_setAttenuationFromPot()
  {
    int pot = 0;
    for (int i = 0; i < 12; i++)
    {
      int p = analogRead(_pinAnalogInput);
      pot += p;  
    
    }
    pot = pot >> 7;
    if (abs(_pot - pot) > 1)
    {
      _pot = pot;
      _dacCtrl->setAttenuation(_pot);
    }
  };

  bool DACVolumeControl::_motorised()
  {
     return (_pinMotor1 != 255) && (_pinMotor2 != 255);
  }