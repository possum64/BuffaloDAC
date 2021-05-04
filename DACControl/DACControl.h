/*
  Class library for controlling one or more DACs
*/
 
//#define USE_ES9018

#include <global.h>
#include <SerialHelper.h>
#include <StackList.h>
#ifdef USE_ES9018
  #include <ES9018.h>
#endif
#include <ES9028.h>

#ifndef DACControl_h
#define DACControl_h
#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
  #include "pins_arduino.h"
  #include "WConstants.h"
#endif

class DACControl 
{
  public:
    typedef void (*EventFunction) ();
    #ifdef USE_ES9018
      typedef bool (*ES9018Function) (ES9018* dac);
    #endif
    typedef bool (*ES9028Function) (ES9028* dac);
    enum Input{I2S, SPDIF};

    #ifdef USE_ES9018
      DACControl(ES9018 es9018dacs[], byte es9018dacCount, ES9028 es9028dacs[], byte es9028dacCount);
    #endif
    DACControl(ES9028 es9028dacs[], byte es9028dacCount);
    DACControl(ES9028 es9028dacs[], byte es9028dacCount, int clockStretchLimit, byte addr = 255);
    #ifdef USE_ES9018
      void initES9018(ES9018Function val);
    #endif
    void initES9028(ES9028Function val);
    void onBeforePowerOff(EventFunction val);
    void onAfterPowerOff(EventFunction val);
    void onBeforePowerOn(EventFunction val);
    void onAfterPowerOn(EventFunction val);
    void onMuted(EventFunction val);
    void onUnmuted(EventFunction val);
    void onLock(EventFunction val);
    void onLockReadError(EventFunction val);
    void onNoLock(EventFunction val);
    void onInitialised(EventFunction val);
    void onNotInitialised(EventFunction val);
    void onAutomuteStatusChanged(EventFunction val);
    void begin();
    void powerOn();
    void powerOff();
    DACControl::Input getInput();
    bool getPower();
    void togglePower();
    void mute();
    bool automuted();
    bool initialised();
    void unmute();
    void setSPDIF();
    void setUSB();
    void setNoI2C();
    void toggleInput();
    void loop();
    int locked();
    void setAttenuation(byte val);
    void setFilterShape(ES9028::FilterShape val);
    void setPinDACReset(byte val);
    void setPinPowerRelay(byte val);
    void setPinSDA(byte val);
    void setPinSCL(byte val);

  private: 

     const int _initDelay = 1500;                          // delay before attempting to initialise DACs after poweron
     const unsigned int _delayUnmute = 250;                // wait for AVB to properly lock onto stream
     const unsigned int _lockSampleInterval = 250;         // lock sample interval in ms
     byte _pinPowerRelay = 255;
     byte _pinDACReset = 255;
     byte _pinSDA = 255;
     byte _pinSCL = 255;
     byte _addrI2C;

     #ifdef USE_ES9018
     ES9018 *_es9018dacs = NULL;
     byte _es9018dacCount = 0;
     ES9018Function _initES9018;
     #endif
     ES9028 *_es9028dacs = NULL;
     byte _es9028dacCount = 0;
     boolean _power = false;
     boolean _automuted = false;
     boolean _inputSPDIF = false;
     unsigned long _lastPowerOnEvent = 0;                 // last time power to DACs turned on
     unsigned long _previousLockSampleMillis = 0;         // last time lock status was sampled
     unsigned long _previousPowerLightMillis = 0;         // last time power light toggled
     int _prevSignalLock = -1;                            // previous signal lock state
     int _clockStretchLimit = -1;
     boolean _initialised = false;
     boolean _errorInitialising = false;
     ES9028Function _initES9028;
     EventFunction _onLock;
     EventFunction _onLockReadError;
     EventFunction _onNoLock;
     EventFunction _onBeforePowerOn;
     EventFunction _onAfterPowerOn;
     EventFunction _onBeforePowerOff;
     EventFunction _onAfterPowerOff;
     EventFunction _onInitialised;
     EventFunction _onNotInitialised;
     EventFunction _onAutomuteStatusChanged;

     int _allLockedValue();
     void _eventInitialised();
     void _eventAutomuteStatusChange();
     boolean _initSuccess();
     void _eventBeforePowerOn();
     void _eventAfterPowerOn();
     void _eventBeforePowerOff();
     void _eventAfterPowerOff();
     void _disableDACs();
     void _enableDACs();
     void _initDACs();
     void _initFail(String name);

};

#endif
