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
  
void DACControl::onMuted(EventFunction val)
{
  _onMuted = val;
};
  
void DACControl::onUnmuted(EventFunction val)
{
  _onUnmuted = val;
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
    Serial.println(F("Power On"));
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
      Serial.println(F("Power Off"));
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
    Serial.println(F("toggle power"));
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
    if (getPower())
    {
      _muted = true;
      Serial.println(F("Mute"));
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
      if (_onMuted != NULL)
        (_onMuted());
   }
  };

  bool DACControl::muted()
  {
    return _muted;
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
    if (getPower())
    {
      Serial.println(F("Unmute"));
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
      _muted = false;
      if (_onUnmuted != NULL)
        (_onUnmuted());
    }
  };
    
  void DACControl::setSPDIF()
  {
    if (getPower())
    {
      Serial.println(F("SPDIF Input"));
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
      Serial.println(F("USB Input"));
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
      Serial.println(F("No I2C"));
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
    Serial.println(F("toggle input"));
    if (_inputSPDIF)
    {
      setUSB();
    }
    else
    {
      setSPDIF();
    }
  };
  
  void DACControl::evaluate()
  {
    if (_power)
    {
      if (!_initialised)
      {
        if ((millis() - _lastPowerOnEvent) >= _initDelay)
          _initDACs();
      }
      else if (_initSuccess())
      {
      if (millis() - _previousLockSampleMillis >= _lockSampleInterval)
      {
        _previousLockSampleMillis = millis();
        bool automuted = _es9028dacCount > 0;
        for (int i = 0; (i < _es9028dacCount) && (automuted); i++)
        {
          automuted = _es9028dacs[i].automuted();
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
               Serial.print(_es9018dacs[i].getName());
               if (_es9018dacs[i].locked())
                 Serial.println(F(" DAC locked"));
               else
                 Serial.println(F(" DAC not locked"));
            }
            #endif
            for (int i=0; i < _es9028dacCount; i++)
            {
               Serial.print(_es9028dacs[i].getName());
               if (_es9028dacs[i].locked())
                 Serial.println(F(" DAC locked"));
               else
                 Serial.println(F(" DAC not locked"));
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
        Serial.print(F("Error reading Lock: "));
        Serial.println(_es9018dacs[i].getName());
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
        Serial.print(F("Error reading Lock: "));
        Serial.println(_es9028dacs[i].getName());
        result += (1 << (i + offset + 8));
      }
   }
   return result;
  };

  void DACControl::setAttenuation(byte attenuation)
  {
    _attenuation = attenuation;
    Serial.print(F("Setting attenuation to: "));
    Serial.println(_attenuation);
    #ifdef USE_ES9018
    for (int i=0; i < _es9018dacCount; i++)
    {
        _es9018dacs[i].setAttenuation(attenuation);
    }
    #endif
    for (int i=0; i < _es9028dacCount; i++)
    {
       _es9028dacs[i].setVolume1(attenuation);
    }
  };
  
  void DACControl::setFilterShape(ES9028::FilterShape val)
  {
    #ifdef USE_ES9018
    for (int i=0; i < _es9028dacCount; i++)
    {
      _es9028dacs[i].setFilterShape(static_cast<byte>(val) + 1);
    }
    #endif
  };
    
  void DACControl::setPinDACReset(byte val)
  {
    _pinDACReset = val;
    if (_pinDACReset != 255)
      pinMode(_pinDACReset, OUTPUT);
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
    Serial.println(F("Initialisation OK"));
    if (_onInitialised != NULL)
      _onInitialised();
    delay(1000);
    unmute();
  }
  else
  {
    Serial.println(F("Initialisation Failed"));
    if (_onNotInitialised != NULL)
      _onNotInitialised();
  }
};
    
void DACControl::_eventAutomuteStatusChange()
{ 
  Serial.println(F("Automute status change detected"));
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
  _initialised = false;
  if (_onAfterPowerOn != NULL)
    _onAfterPowerOn();
};
    
void DACControl::_eventBeforePowerOff()
{
  mute();
  _disableDACs();
  if (_onBeforePowerOff != NULL)
    _onBeforePowerOff();
  delay(1000);
};
    
    void DACControl::_eventAfterPowerOff()
    {
      Serial.println(F("Resetting..."));
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
      TWCR = 0; // reset TwoWire Control Register to default, inactive state 
      //soft_restart(); //call reset
      if (_onAfterPowerOff != NULL)
        _onAfterPowerOff();
    };
    
    void DACControl::_disableDACs()
    {
      Serial.println(F("Pull DAC Reset Pin Low (disable)"));
      digitalWrite(_pinDACReset, LOW);  // put dacs into reset
    };
    
    void DACControl::_enableDACs()
    {
      Serial.println(F("Pull DAC Reset Pin High (enable)"));
      digitalWrite(_pinDACReset, HIGH);  // put dacs out of reset
    }
    
    void DACControl::_initDACs()
    {
      Serial.println(F("Initialising DACs"));
      _enableDACs();
        // set the SDA and SCL pins (A4, A5 respectively) for I2S comms on the 8266 and join the I2C bus as a master
      Wire.begin();
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
            Serial.print(F("Initialising ES8018 DAC at address "));
            Serial.println(String(_es9018dacs[i].getAddress(), HEX));
            if (_es9018dacs[i].initialise())
            {
              _es9018dacs[i].mute();
              Serial.print(F("Found DAC at address "));
              Serial.print(String(_es9018dacs[i].getAddress(), HEX));
              Serial.print(F(" after "));
              Serial.print(String(millis() - _lastPowerOnEvent));
              Serial.println(F(" milliseconds"));
              if (_initES9018 == NULL)
                Serial.println(F("no DAC initialisation specified"));
              else
                if (!_initES9018(&_es9018dacs[i]))
                  _es9018dacs[i].reset();
            }
            else
            {
              Serial.print(F("! DAC"));
              Serial.print(String(_es9018dacs[i].getAddress(), HEX));
              Serial.print(F(" not found after "));
              Serial.print(String(millis() - _lastPowerOnEvent));
              Serial.println(F(" milliseconds"));
            }
           }
        }
        #endif
        for (int i=0; i < _es9028dacCount; i++)
        {
          if (!_es9028dacs[i].getInitialised())
          {
            Serial.print(F("Initialising ES9028 DAC at address "));
            Serial.println(String(_es9028dacs[i].getAddress(), HEX));
            // try communicating with DAC
            if (_es9028dacs[i].initialise())
            {
              _es9028dacs[i].mute();
              Serial.print(F("Found DAC at address "));
              Serial.print(String(_es9028dacs[i].getAddress(), HEX));
              Serial.print(F(" after "));
              Serial.print(String(millis() - _lastPowerOnEvent));
              Serial.println(F(" milliseconds"));
              if (_initES9028 == NULL)
                Serial.println(F("no DAC initialisation specified"));
              else
                if (!_initES9028(&_es9028dacs[i]))
                  _es9028dacs[i].reset();
            }
            else
            {
              Serial.print(F("! DAC"));
              Serial.print(String(_es9028dacs[i].getAddress(), HEX));
              Serial.print(F(" not found after "));
              Serial.print(String(millis() - _lastPowerOnEvent));
              Serial.println(F(" milliseconds"));
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
          _initFail(_es9028dacs[i].getName());
     }
      _initialised = true;
      _eventInitialised();
    };
    
    void DACControl::_initFail(String name)
    {
        Serial.print(name);
        Serial.print(F(" DAC failed to initialise after "));
        Serial.print(String(millis() - _lastPowerOnEvent));
        Serial.println(F(" milliseconds"));
    };

