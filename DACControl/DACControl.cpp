#include "DACControl.h"



#ifdef USE_ES9018
   DACControl::DACControl(ES9018 es9018dacs[], byte es9018dacCount, ES9028 es9028dacs[], byte es9028dacCount)
   {
     _es9018dacs = es9018dacs;
     _es9018dacCount = es9018dacCount;
     _es9028dacs = es9028dacs;
     _es9028dacCount = es9028dacCount;
   };
#endif

DACControl::DACControl(ES9028 es9028dacs[], byte es9028dacCount)
{
  _es9028dacs = es9028dacs;
  _es9028dacCount = es9028dacCount;
};

DACControl::DACControl(ES9028 es9028dacs[], byte es9028dacCount, int clockStretchLimit, byte addrI2C)
{
  _es9028dacs = es9028dacs;
  _es9028dacCount = es9028dacCount;
  _clockStretchLimit = clockStretchLimit;
  _addrI2C = addrI2C;
};

#ifdef USE_ES9018
  void DACControl::initES9018(ES9018Function val)
  {
    _initES9018 = val;
  };
#endif
  
void DACControl::initES9028(ES9028Function val)
{
  _initES9028 = val;
}
  
void DACControl::onBeforePowerOff(EventFunction val)
{
  _onBeforePowerOff = val;
};
  
void DACControl::onAfterPowerOff(EventFunction val)
{
  _onAfterPowerOff = val;
};
  
void DACControl::onBeforePowerOn(EventFunction val)
{
  _onBeforePowerOn = val;
};
  
void DACControl::onAfterPowerOn(EventFunction val)
{
  _onAfterPowerOn = val;
};
  
void DACControl::onLock(EventFunction val)
{
  _onLock = val;
};
  
void DACControl::onNoLock(EventFunction val)
{
  _onNoLock = val;
};
  
void DACControl::onLockReadError(EventFunction val)
{
  _onLockReadError = val;
};
  
void DACControl::onInitialised(EventFunction val)
{
  _onInitialised = val;
};
  
void DACControl::onNotInitialised(EventFunction val)
{
  _onNotInitialised = val;
};

void DACControl::onAutomuteStatusChanged(EventFunction val)
{
  _onAutomuteStatusChanged = val;
};

void DACControl::powerOn()
{
  if (!_power)
  {
    _eventBeforePowerOn();
    _power = true;
    Msg::println(F("Power On"));
    if (_pinPowerRelay != 255)
      digitalWrite(_pinPowerRelay, HIGH);
    _eventAfterPowerOn();
  }
};
    
  void DACControl::powerOff()
  {
    if (_power)
    {
      _eventBeforePowerOff();
      if (_pinPowerRelay != 255)
        digitalWrite(_pinPowerRelay, LOW);
      _power = false;
      Msg::println(F("Power Off"));
      _eventAfterPowerOff();
    }
  };
    
  DACControl::Input DACControl::getInput()
  {
    if (_inputSPDIF)
      return SPDIF;
    else
      return I2S;
  };

  bool DACControl::getPower()
  {
    return _power;
  };
  
  void DACControl::togglePower()
  {
    Msg::println(F("toggle power"));
    if (_power)
    {
      powerOff();
    }
    else
    {
      powerOn();
    }
  };
  
  void DACControl::mute()
  {
    if (initialised() && !_errorInitialising)
    {
      Msg::println(F("Mute"));
      #ifdef USE_ES9018
      for (int i = 0; i < _es9018dacCount; i++)
      {
        _es9018dacs[i].mute();
      }
      #endif
      for (int i = 0; i < _es9028dacCount; i++)
      {
        _es9028dacs[i].mute();
      }
    }
  };

 bool DACControl::automuted()
  {
    return _automuted;
  };
  
  bool DACControl::initialised()
  {
    return _initialised;
  }
  ;
  void DACControl::unmute()
  {
    if (initialised() && !_errorInitialising)
    {
      Msg::println(F("Unmute"));
      #ifdef USE_ES9018
      for (int i = 0; i < _es9018dacCount; i++)
      {
        _es9018dacs[i].unmute();
      }
      #endif
      for (int i=0; i < _es9028dacCount; i++)
      {
        _es9028dacs[i].unmute();
      }
    }
  };
    
  void DACControl::setSPDIF()
  {
    if (getPower())
    {
      Msg::println(F("SPDIF Input"));
      #ifdef USE_ES9018
      for (int i = 0; i < _es9018dacCount; i++)
      {
        _es9018dacs[i].setInputSelect(ES9018::SPDIF);
      }
      #endif
      for (int i=0; i < _es9028dacCount; i++)
      {
        _es9028dacs[i].setInputSelect(ES9028::InputSelect_SPDIF);
      }
      _inputSPDIF = true;
    }
  };
  
  void DACControl::setUSB()
  {
    if (getPower())
    {
      Msg::println(F("USB Input"));
      #ifdef USE_ES9018
      for (int i = 0; i < _es9018dacCount; i++)
      {
        _es9018dacs[i].setInputSelect(ES9018::I2SorDSD);
      }
      #endif
      for (int i=0; i < _es9028dacCount; i++)
      {
        _es9028dacs[i].setInputSelect(ES9028::InputSelect_SERIAL);
      }
      _inputSPDIF = false;
    }
  };
  
  void DACControl::setNoI2C()
  {
      Msg::println(Msg::W, F("Set No I2C"));
      #ifdef USE_ES9018
      for (int i = 0; i < _es9018dacCount; i++)
      {
        _es9018dacs[i].noI2C = true;
      }
      #endif
      for (int i=0; i < _es9028dacCount; i++)
      {
        _es9028dacs[i].noI2C = true;
      }
  };
  
  void DACControl::toggleInput()
  {
    Msg::println(F("toggle input"));
    if (_inputSPDIF)
    {
      setUSB();
    }
    else
    {
      setSPDIF();
    }
  };
  
  void DACControl::loop()
  {
    if (_power)
    {
      if (!initialised())
      {
        if ((millis() - _lastPowerOnEvent) >= _initDelay)
          _initDACs();
      }
      else if (_initSuccess())
      {
      if (millis() - _previousLockSampleMillis >= _lockSampleInterval)
      {
        _previousLockSampleMillis = millis();
        bool automuted = (_es9028dacCount > 0);
        bool b;
        for (int i = 0; i < _es9028dacCount; i++)
        {
          if (_es9028dacs[i].getAutomuted(b))
          {
            if (!b)
            {
              // automuted is not true if any DAC is not automuted
              automuted = false;
            }
          }  
          else
          {
            // I2C error reading Automuted flag
            Msg::print(Msg::E, F("Error reading Automute: "));
            Msg::println(Msg::E, _es9028dacs[i].getName());
          }
        }
        // Detect automute status change
        if (automuted != _automuted)
        {
           _automuted = automuted;
           _eventAutomuteStatusChange();
        }
        // Detect lock status
        int signalLock = locked();
        if (signalLock != _prevSignalLock)
        {
          if (signalLock == _allLockedValue())
          {
            if (_onLock != NULL)
              _onLock();
          }
          else
          {
            if (signalLock < 256)
            {
              if (_onNoLock != NULL)
                _onNoLock();
            }
            else
              {
                // I2C error reading lock
                if (_onLockReadError != NULL)
                  _onLockReadError();
             }
            }
            _prevSignalLock = signalLock;
            #ifdef USE_ES9018
            for (int i=0; i < _es9018dacCount; i++)
            {
               Msg::print(_es9018dacs[i].getName());
               if (_es9018dacs[i].locked())
                 Msg::println(F(" DAC locked"));
               else
                 Msg::println(F(" DAC not locked"));
            }
            #endif
            for (int i=0; i < _es9028dacCount; i++)
            {
               Msg::print(_es9028dacs[i].getName());
               if (_es9028dacs[i].locked())
                 Msg::println(F(" DAC locked"));
               else
                 Msg::println(F(" DAC not locked"));
            }
          }
        }
      }
    }
  };
  
  int DACControl::locked()
  {
   int result = 0; 
   int offset = 0;
   bool readError;
   #ifdef USE_ES9018
   offset = _es9018dacCount;
   for (int i=0; i < _es9018dacCount; i++)
   {
      if (_es9018dacs[i].locked(readError))
        result += (1 << i);
      else if (readError)
      {
        Msg::print(Msg::E, F("Error reading Lock: "));
        Msg::println(Msg::E, _es9018dacs[i].getName());
        result += (1 << (i + 8));
      }
   }
   #endif
   for (int i=0; i < _es9028dacCount; i++)
   {
      if (_es9028dacs[i].locked(readError))
          result += (1 << (i + offset));
      else if (readError)
      {
        Msg::print(Msg::E, F("Error reading Lock: "));
        Msg::println(Msg::E, _es9028dacs[i].getName());
        result += (1 << (i + offset + 8));
      }
   }
   return result;
  };

  void DACControl::setAttenuation(byte val)
  {
    if (val > 124)
    val = 124;
    Msg::print(F("Setting attenuation to: "));
    Msg::println(val);
    #ifdef USE_ES9018
    for (int i=0; i < _es9018dacCount; i++)
    {
        _es9018dacs[i].setAttenuation(val);
    }
    #endif
    for (int i=0; i < _es9028dacCount; i++)
    {
       _es9028dacs[i].setVolume1(val);
    }
  };
  
  void DACControl::setFilterShape(ES9028::FilterShape val)
  {
    for (int i=0; i < _es9028dacCount; i++)
    {
      _es9028dacs[i].setFilterShape(val);
    }
  };
    
  void DACControl::setPinDACReset(byte val)
  {
    _pinDACReset = val;
    if (_pinDACReset != 255)
    {
      pinMode(_pinDACReset, OUTPUT);
      pinMode(2, OUTPUT);
    }
  };
  
  void DACControl::setPinPowerRelay(byte val)
  {
    _pinPowerRelay = val;
    if (_pinPowerRelay != 255)
    {
      digitalWrite(_pinPowerRelay, LOW);
      pinMode(_pinPowerRelay, OUTPUT);
    }
  };
  
  void DACControl::setPinSDA(byte val)
  {
    _pinSDA = val;
  };
  
  void DACControl::setPinSCL(byte val)
  {
    _pinSCL = val;
  };
  
int DACControl::_allLockedValue()
{
 int result = 0; 
 int offset = 0;
 #ifdef USE_ES9018
   offset =_es9018dacCount;
   for (int i=0; i < _es9018dacCount; i++)
   {
      result += (1 << i);
   }
 #endif
 for (int i=0; i < _es9028dacCount; i++)
 {
    result += (1 << (i + offset));
 }
 return result;
};
    
void DACControl::_eventInitialised()
{
  if (_initSuccess())
  {
    Msg::println(F("Initialisation OK"));
    if (_onInitialised != NULL)
      _onInitialised();
  }
  else
  {
    if (_onNotInitialised != NULL)
      _onNotInitialised();
  }
};
    
void DACControl::_eventAutomuteStatusChange()
{ 
  Msg::println(F("Automute status change detected"));
  if (_onAutomuteStatusChanged != NULL)
    _onAutomuteStatusChanged();
};
    
boolean DACControl::_initSuccess()
{
  #ifdef USE_ES9018
     for (int i=0; i < _es9018dacCount; i++)
     {
        if (!_es9018dacs[i].getInitialised())
          return false;
     }
  #endif
  for (int i=0; i < _es9028dacCount; i++)
  {
     if (!_es9028dacs[i].getInitialised())
       return false;
  }
  return true;
};

void DACControl::_eventBeforePowerOn()
{
  if (_onBeforePowerOn != NULL)
    _onBeforePowerOn();
};
    
void DACControl::_eventAfterPowerOn()
{
  _lastPowerOnEvent = millis();
  if (_onAfterPowerOn != NULL)
    _onAfterPowerOn();
};
    
void DACControl::_eventBeforePowerOff()
{
   mute();
   delay(10);
  _disableDACs();
  if (_onBeforePowerOff != NULL)
    _onBeforePowerOff();
};
    
    void DACControl::_eventAfterPowerOff()
    {
      Msg::println(Msg::W, F("Resetting..."));
      #ifdef USE_ES9018
      for (int i=0; i < _es9018dacCount; i++)
      {
         _es9018dacs[i].reset();
      }
      #endif
      for (int i=0; i < _es9028dacCount; i++)
      {
         _es9028dacs[i].reset();
      }
      _initialised = false;
      _prevSignalLock = -1;
      //TWCR = 0; // reset TwoWire Control Register to default, inactive state 
      //soft_restart(); //call reset
      if (_onAfterPowerOff != NULL)
        _onAfterPowerOff();
    };
    
    void DACControl::_disableDACs()
    {
      Msg::println(F("Pull DAC Reset Pin Low (disable)"));
      digitalWrite(_pinDACReset, LOW);  // put dacs into reset
      digitalWrite(2, LOW);  // put dacs into reset
    };
    
    void DACControl::_enableDACs()
    {
      Msg::println(F("Pull DAC Reset Pin High (enable)"));
      digitalWrite(_pinDACReset, HIGH);  // put dacs out of reset
      digitalWrite(2, HIGH);  // put dacs out of reset
    }
    
    void DACControl::begin()
    {
        // set the SDA and SCL pins (A4, A5 respectively) for I2S comms on the 8266 and join the I2C bus as a master
      Msg::print(F("Joining I2C Bus as master"));
      if ((_pinSDA != 255) && (_pinSCL != 255))
      {
        if (_addrI2C == 255)
        {
          Wire.begin(_pinSDA, _pinSCL);
          Msg::println();
        }
        else
        {
          Wire.begin(_pinSDA, _pinSCL, _addrI2C);
          Msg::print(F(" and slave at address "));
          Msg::println(String(_addrI2C, HEX));
        }
      }
      else
      {
        if (_addrI2C == 255)
        {
           Wire.begin();
           Msg::println();
        }
        else
        {
          Wire.begin(_addrI2C);
          Msg::print(F(" and slave at address "));
          Msg::println(String(_addrI2C, HEX));
        }
      }
      if (_clockStretchLimit != -1)
      {
        Wire.setClockStretchLimit(_clockStretchLimit);    // in µs
      }
    }

    void DACControl::_initDACs()
    {
      Msg::println(F("Initialising DACs"));
      _errorInitialising = false;
      _enableDACs();
      bool retry = true;
      while (!_initSuccess() && retry)
      {
       retry = false; // change to attempt multiple retries
       // attempt to initialise uninitialised dacs
       #ifdef USE_ES9018
       for (int i=0; i < _es9018dacCount; i++)
        {
           if (!_es9018dacs[i].getInitialised())
           {
            // try communicating with DAC
            Msg::print(F("Initialising ES8018 DAC at address "));
            Msg::println(String(_es9018dacs[i].getAddress(), HEX));
            if (_es9018dacs[i].initialise())
            {
              _es9018dacs[i].mute();
              Msg::print(F("Found DAC at address "));
              Msg::print(String(_es9018dacs[i].getAddress(), HEX));
              Msg::print(F(" after "));
              Msg::print(String(millis() - _lastPowerOnEvent));
              Msg::println(F(" milliseconds"));
              if (_initES9018 == NULL)
                Msg::println(Msg::W, F("no DAC initialisation specified"));
              else
                if (!_initES9018(&_es9018dacs[i]))
                  _es9018dacs[i].reset();
            }
          }
        }
        #endif
        for (int i=0; i < _es9028dacCount; i++)
        {
          if (!_es9028dacs[i].getInitialised())
          {
            Msg::print(F("Initialising ES9028 DAC at address "));
            Msg::println(String(_es9028dacs[i].getAddress(), HEX));
            // try communicating with DAC
            if (_es9028dacs[i].initialise())
            {
              _es9028dacs[i].mute();
              Msg::print(F("Found DAC at address "));
              Msg::print(String(_es9028dacs[i].getAddress(), HEX));
              Msg::print(F(" after "));
              Msg::print(String(millis() - _lastPowerOnEvent));
              Msg::println(F(" milliseconds"));
              if (_initES9028 == NULL)
                Msg::println(Msg::W, F("no DAC initialisation specified"));
              else
                if (!_initES9028(&_es9028dacs[i]))
                  _es9028dacs[i].reset();
              _es9028dacs[i].unmute();
            }
          }
        }
      }
     #ifdef USE_ES9018
     for (int i=0; i < _es9018dacCount; i++)
     {
        if (!_es9018dacs[i].getInitialised())
          _initFail(_es9018dacs[i].getName());
     }
     #endif
     for (int i=0; i < _es9028dacCount; i++)
     {
        if (!_es9028dacs[i].getInitialised())
        {
          _initFail(_es9028dacs[i].getName());
          _errorInitialising = true;
        }
      }
      _initialised = true;
      _eventInitialised();
    };
    
    void DACControl::_initFail(String name)
    {
        Msg::print(Msg::E, name);
        Msg::print(Msg::E, F(" DAC failed to initialise after "));
        Msg::print(Msg::E, String(millis() - _lastPowerOnEvent));
        Msg::println(Msg::E, F(" milliseconds"));
    };

