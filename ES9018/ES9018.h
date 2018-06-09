/*
  Class library for controlling the ES9018 Sabre32 DAC from an Arduino via I2C 

  In essence, this library repackages much of the code from the HifiDuino Sabre32 project (see https://hifiduino.wordpress.com/sabre32/)
  into a C++ class in order to make it more 'user friendly' by abstracting the programmer from having to deal with the underlying 
  bits 'n' bytes of the DAC's registry settings.

  All DAC registry writes can be viewed via the Arduino's serial monitor

  (see the WIRE library for details on connecting an I2C device to an Arduino board. Be aware that most I2C devices use 3.3 volts!)
*/

#include <Wire.h>

#ifndef ES9018_h
#define ES9018_h
#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
  #include "pins_arduino.h"
  #include "WConstants.h"
#endif

class ES9018 
{
  public:
    enum Mode{MonoLeft, MonoRight, Stereo, EightChannel};
    enum Phase{InPhase, AntiPhase};
    enum Clock{Clock100Mhz, Clock80Mhz};
    enum InputSelect{SPDIF, I2SorDSD};
    enum DeEmphasis{DeEmph32k, DeEmph441k, DeEmph48k};
    enum NotchDelay{NoDelay=0, Delay4=1, Delay8=2, Delay16=3, Delay32=4, Delay64=5};
    enum Quantizer{SixBit=0, SevenBit=1, EightBit=2, NineBit=3};
    enum DPLLBandwidth{None=0, Lowest=1, Low=2, MediumLow=3, Medium=4, MedHigh=5, High=6, Highest=7};
    enum DPLLMode{AllowAll, UseBest};
    enum DPLL128Mode{UseDPLLSetting, MultiplyBy128};
    enum FIR_RollOffMode{Slow, Fast}; 
    enum FIR_Coefficients{FIR27, FIR28}; 
    enum IIR_Bandwidth{IIR_Normal, IIR_50k, IIR_60k, IIR_70k}; 
    enum SPDIFMode{SPDIF_Auto, SPDIF_Manual};

    // default to 8 channel mode with default phase settings and default I2C address 0x48
    ES9018(String name, Clock value);  
    // specify mode with default phase settings and default I2C address 0x49 for right mono and 0x48 otherwise
    ES9018(String name, Clock value, Mode mode);  
    // specify whether 8 channel, stereo, or mono left/right and channel phase with default I2C address 0x49 for right mono and 0x48 otherwise
    ES9018(String name, Clock value, Mode mode, Phase oddChannels, Phase evenChannels);   
    // specify whether 8 channel, stereo, or mono left/right, channel phase and custom I2C address
    ES9018(String name, Clock value, Mode mode, Phase oddChannels, Phase evenChannels, byte address); 

    bool noI2C = false;
    bool initialise();                                     //  writes all register values for the first time. After an init() any further register changes are written immediately
    bool getInitialised();
    void reset();
    bool validSPDIF(bool &status);
    ES9018::Mode getMode();
    bool locked();
    bool locked(bool &readError);
    byte getAddress();                               // returns the I2C address
    String getName();
    bool mute();
    bool unmute();
    unsigned long sampleRate();
    bool setAttenuation(byte attenuation);
    bool setAutoMuteLevel(byte level);
    bool setBypassOSF(boolean value);
    bool setDeEmphasis(DeEmphasis mode);
    bool setDPLLMode(DPLLMode mode);
    bool setDPLL(DPLLBandwidth bandwidth);
    bool setDPLL128Mode(DPLL128Mode mode);
    bool setFIRRollOff(FIR_RollOffMode mode);
    bool setFIRPhase(Phase phase);
    bool setInputSelect(InputSelect mode);
    bool setIIRBandwidth(IIR_Bandwidth value);
    bool setJitterReduction(boolean value);
    bool setJitterReductionBypass(boolean value);
    bool setQuantizer(Quantizer value);
    bool setNotchDelay(NotchDelay value);
    bool setPhaseB(Phase value);
    bool setSPDIFMode(SPDIFMode mode);
    bool setSPDIFAutoDeEmphasis(boolean value);

  private:
    bool _locked(bool &status);
    String _name;
    Mode _mode = EightChannel;   // default to eight channel mode
    byte _address = 0x48;           // set default I2C address
    Clock _clock = Clock100Mhz;  // set default clock speed to 100Mhz
    bool _initialised = false;
    const int _readRetryInterval = 20;    // _readRegister retry interval
    Phase _oddChannels = InPhase;
    Phase _evenChannels = InPhase;

    /*
    Register 8 (0x08) Auto-mute level, manual spdif/i2s
      |0| | | | | | | |  Use I2S or DSD (D)
      |1| | | | | | | |  Use SPDIF
      | |1|1|0|1|0|0|0|  Automute trigger point (D)

     I2S/DSD input:
     01101000 (0x68)
     SPDIF input:
     11101000 (0xE8) 
 
     NOTE on auto/manual SPDIF selection
     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     Apparently, auto detection of spdif and I2S (with I2S/DSD enabled in reg 8), only works when the DAC
     powers up. Once you turn off auto-SPDIF and then turn on auto-SPDIF, it will not work with SPDIF input while
     reg 8 is still set for I2S/DSD. In order for SDPIF to work, reg 8 must be set for SPDIF and reg 17 for auto-
     SDPIF. In summary:
 
      reg 17 auto-SPDIF | reg 8 source | Result
      ~~~~~~~~~~~~~~~~~~|~~~~~~~~~~~~~~|~~~~~~~
        ON              | I2S/DSD      | SPDIF ON: Only works when DAC starts with these (default) settings
        ON              | SPDIF        | SPDIF ON: Works even when auto-SPDIF is set to OFF, then to ON (1)
        OFF             | SPDIF        | SPDIF OFF, I2S OFF (3)
        OFF             | I2S/DSD      | I2S ON (2)
    
      Thus for manual operation, use (1) to select SDPIF; use (2) to select I2S. I don't know what is the
      purpose of (3)
    */

    //byte _reg10 = 0xCE;               // jitter ON, dacs unmute, other defaults;
    /*
     Register 10 (0x0A) (MC2)
      | | | | |1| | | |  jitter reduction bypass on
      | | | | | |1| | |  jitter reduction on
      | | | | | | | |1|  mute
      | | | | | | | |0|  unmute
    */
 
    //byte _reg11 = B10001110;               
    /*
     Register 11 (0x0B) (MC2)
      |1|0|0| | | | | |  Reserved, Must be 100 (D)
      | | | |0|0|0| | |  DPLL BW: No Bandwidth
      | | | |0|0|1| | |  DPLL BW: Lowest (D)
      | | | |0|1|0| | |  DPLL BW: Low
      | | | |0|1|1| | |  DPLL BW: Medium-Low 
      | | | |1|0|0| | |  DPLL BW: Medium 
      | | | |1|0|1| | |  DPLL BW: Medium-High
      | | | |1|1|0| | |  DPLL BW: High 
      | | | |1|1|1| | |  DPLL BW: Highest 
      | | | | | | |0|0|  DeEmph: 32k
      | | | | | | |0|1|  DeEmph: 44.1k (D)
      | | | | | | |1|0|  DeEmph: 48k
      | | | | | | |1|1|  Reserved, do not use

      Lowest Bandwidth 44.1K De-emphasis select (these are chip default):
      10000101 or 0x85 (or decimal 133)
    */

    //byte _reg12 = 0x1F;  // default to n/64 notch delay          
    /*
    Register 12 (0x0C) Notch Delay
    |0|x|x|x|x|x|x|x| Dither Control: Apply
    |1|x|x|x|x|x|x|x| Dither Control: Use fixed rotation pattern
    |x|0|x|x|x|x|x|x| Rotator Input: NS-mod input
    |x|1|x|x|x|x|x|x| Rotator Input: External input
    |x|x|0|x|x|x|x|x| Remapping: No remap
    |x|x|1|x|x|x|x|x| Remapping: Remap DIG outputs for “max phase separation in analog cell”

    |x|x|x|0|0|0|0|0| No Notch
    |x|x|x|0|0|0|0|1| Notch at MCLK/4
    |x|x|x|0|0|0|1|1| Notch at MCLK/8
    |x|x|x|0|0|1|1|1| Notch at MCLK/16
    |x|x|x|0|1|1|1|1| Notch at MCLK/32
    |x|x|x|1|1|1|1|1| Notch at MCLK/64

    |0|0|1|0|0|0|0|0| Power-on Default
    */

    //byte _reg13 = 0x00;               
    /*
    Register 13 (0x0D) DAC polarity
    In-phase: 0x00 (all 8 channels) (D)
    Anti-phase: 0xFF (all 8 channels)
    */

    //byte _reg19 = 0x00;               
    /*
    Register 19 (0x13) DACB polarity
    In-phase: 0xFF (all 8 channels)
    Anti-phase: 0x00 (all 8 channels) (D)
    */

   //byte _reg14 = 0xF9;               
    /*
    Set up for I2S/DSD support according to BuffII input wiring
 
     Register 14 (0x0E) DAC source, IIR Bandwidth and FIR roll off
      |0| | | | | | | | Source of DAC8 is DAC8 (D)
      |1| | | | | | | | Source of DAC8 is DAC6
      | |0| | | | | | | Source of DAC7 is DAC7 (D)
      | |1| | | | | | | Source of DAC7 is DAC5
      | | |0| | | | | | Source of DAC4 is DAC4 (D)
      | | |1| | | | | | Source of DAC4 is DAC2
      | | | |0| | | | | Source of DAC3 is DAC3 (D)
      | | | |1| | | | | Source of DAC3 is DAC1
      | | | | |1| | | | Reserved, must be 1 (D)
      | | | | | |0|0| | IIR Bandwidth: Normal (for PCM)
      | | | | | |0|1| | IIR Bandwidth: 50k (for DSD) (D)
      | | | | | |1|0| | IIR Bandwidth: 60k (for DSD)
      | | | | | |1|1| | IIR Bandwidth: 70k (for DSD)
      | | | | | | | |0| FIR Rolloff: Slow
      | | | | | | | |1| FIR Rolloff: Fast (D)

     For the way Buffalo II input is wired we use the following:
     11111001 or 0xF9 for I2S, normal BW (PCM) and fast roll off (default for BuffII)
    */

    //byte _reg15 = 0x00;    // 6-bit quantizer           

    //byte _reg17 = 0x1C;               
    /*
      Register 17 (0x11) (MC5)
       |1| | | | | | | |  Mono Right (if set for MONO)
       |0| | | | | | | |  Mono Left (if set for MONO) (D)
       | |1| | | | | | |  OSF (Oversample filter) Bypass
       | |0| | | | | | |  Use OSF (D)
       | | |1| | | | | |  Relock Jitter Reduction
       | | |0| | | | | |  Normal Operation Jitter Reduction (D)
       | | | |1| | | | |  SPDIF: Auto deemph ON (D)
       | | | |0| | | | |  SPDIF: Auto deemph OFF
       | | | | |1| | | |  SPDIF Auto (Only if no I2S on pins) (D)
       | | | | |0| | | |  SPDIF Manual (Manually select SPDIF input)
       | | | | | |1| | |  FIR: 28 coefficients (D)
       | | | | | |0| | |  FIR: 27 coefficients
       | | | | | | |1| |  FIR: Phase invert
       | | | | | | |0| |  FIR: Phase NO invert (D)
       | | | | | | | |1|  All MONO (Then select Mono L or R)
       | | | | | | | |0|  Eight channel (D)
    */

    //byte _reg25 = 0;                  
    /*
     Register 25 (0x19): DPLL Mode control
      |0|0|0|0|0|0| | | Reserved, must be zeros (D)
      | | | | | | |0| | DPLL Bandwidth: allow all Settings
      | | | | | | |1| | DPLL Bandwidth: Use best DPLL Settings (D)
      | | | | | | | |0| DPLL128x: Use DPLL setting (D)
      | | | | | | | |1| DPLL128x: Multiply DPLL by 128
 
     Allow all DPLL settings:
     00000000 or 0x00
     Use best DPLL settings:
     00000010 or 0x02
    */

    boolean _readRegister(byte regAddr, byte &regVal); 
    bool _writeRegister(byte regAddr, byte regVal);             // writes the specified register value to the specified DAC register via I2C
    boolean _writeRegisterBits(byte regAddr, String bits); 
    boolean _changeByte(byte &val, String bits);
    boolean _writeMode();
    boolean _writePhase();
    void _setMode(Mode mode);
    void _setPhase(Phase oddChannels, Phase evenChannels);
    void _setInitialised(boolean val);
    void _printDAC();
};

#endif
