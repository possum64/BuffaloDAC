#include "ES9028.h"

ES9028::ES9028(String name)
{
  _name = name;
}

ES9028::ES9028(String name, Mode mode)
{
  _name = name;
  _mode = mode;
}

ES9028::ES9028(String name, byte addr)
{
  _name = name;
  _address = addr;
}

ES9028::ES9028(String name, Mode mode, byte addr)
{
  _name = name;
  _mode = mode;
  _address = addr;
}

String ES9028::getName()
{
  return _name;
}

void ES9028::_printDAC()
{
  _printDAC(Msg::defaultLevel);
}

void ES9028::_printDAC(Msg::Level level)
{
  Msg::print(level, F("->"));
  Msg::print(level, _name);
  Msg::print(level,F(" DAC ["));
  if (_chipType == Chip_ES9028PRO)
    Msg::print(level, F("ES9028 @"));
  else if (_chipType == Chip_ES9038PRO)
    Msg::print(level, F("ES9038 @"));
  else
    Msg::print(level, F("->Unknown @"));
  Msg::print(level, String(_address, HEX));
  Msg::print(level, F("]: "));
}

boolean ES9028::_invalidSetting()
{
    Msg::println(Msg::E, F("Invalid setting"));
    return false;
}

bool ES9028::_readRegister(byte regAddr, byte &regVal) 
{
  if (!_initialised)
  {
    _printDAC();
    Msg::print(Msg::E, F("Uninitialised Error reading status register "));
    Msg::println(Msg::E, String(regAddr));
    return false;
  }
  if (noI2C)
  {
    regVal = 0;
    return true;
  }
  bool retry;
  int readRetries = _readRetries;
  while (true)
  {
    retry = false;
    Wire.beginTransmission(_address); 
    Wire.write(regAddr);           
    byte result;           
    result = Wire.endTransmission();
    if (result == 0) // success
    {
      bool bytesReceived = Wire.requestFrom(_address, 1); // request one byte from address
      if (bytesReceived == 0) // error
      {
        _printDAC(Msg::W);
        Msg::print(Msg::W, F("Zero bytes returned reading status register "));
        Msg::print(Msg::W, String(regAddr));
        Msg::print(Msg::W, F(" after retry count of "));
        Msg::println(Msg::W, String(_readRetries - readRetries + 1));
        if (readRetries == 0)
        {
          return false;
        }
        readRetries--;
        retry = true;
      }
      if (!retry)
      {
        regVal = Wire.read();         // Return the value returned by specified register
        _printDAC(Msg::D);
        Msg::print(Msg::D, F("read value "));
        Msg::print(Msg::D, String(regVal, BIN));
        Msg::print(Msg::D, F(" from register "));
        Msg::println(Msg::D, String(regAddr));
        return true;
      }
    }
    else 
    {
      _printDAC(Msg::E);
      Msg::print(Msg::E, F("Error reading status register "));
      Msg::print(Msg::E, String(regAddr));
      switch (result)
      {
        case 1:
          Msg::println(Msg::E, F(" - data too long to fit in transmit buffer"));
          break;
        case 2:
          Msg::println(Msg::E, F(" - received NACK on transmit of address"));
          break;
        case 3:
          Msg::println(Msg::E, F(" - received NACK on transmit of data"));
          break;
      case 4:
          Msg::println(Msg::E, F(" - received unspecified error"));
          break;
      }
      return false;
    }
  }
}

bool ES9028::_writeRegister(byte regAddr, byte regVal)
{
  if (!_initialised)
  {
    _printDAC(Msg::E);
    Msg::print(Msg::E, F("Uninitialised Error writing status register "));
    Msg::println(Msg::E, String(regAddr));
    return false;
  }
  if (noI2C)
    return true;
  _printDAC(Msg::D);
  Msg::print(Msg::D, F(": Writing "));
  Msg::print(Msg::D, String(regVal, BIN));
  Msg::print(Msg::D, F(" to register "));
  Msg::println(Msg::D, String(regAddr));
  byte readVal;
  bool readOk = _readRegister(regAddr, readVal);
  if (!readOk)
  {
    Msg::println(Msg::E, F("-Read Error reading current register value- "));
    return false;
  }
  else
  {  
    if (readVal == regVal)
      Msg::println(Msg::D, F("-Write value same as register value- "));
    else
    {
      Wire.beginTransmission(_address); 
      Wire.write(regAddr);               // Specifying the address of register
      int result = Wire.write(regVal);   // Writing the value into the register
      Wire.endTransmission();
      readOk = _readRegister(regAddr, readVal); // confirm write
      if (!readOk)
      {
        _printDAC(Msg::E);
        Msg::print(Msg::E, F("-Write Error- "));
        Msg::print(Msg::E, F(" could not read written value from register "));
        Msg::println(Msg::E, String(regAddr));
        return false;
      }
      else
      {
        if (readVal != regVal)
        {
          _printDAC(Msg::E);
          Msg::print(Msg::E, F("-Write Error- "));
          Msg::print(Msg::E, String(readVal, BIN));
          Msg::print(Msg::E, F(" read from register "));
          Msg::println(Msg::E, String(regAddr));
          return false;
        }
        else
          Msg::println(Msg::I, F("-Write Success!- "));
      }
    }
  }
  return true;
}

boolean ES9028::_changeByte(byte &val, String bits)
{
  boolean result = false;
  int l = bits.length();
  if (l != 8)
  {
    Msg::print(Msg::E, F("changebits: "));
    Msg::print(Msg::E, bits);
    Msg::println(Msg::E, F(" must contain 8 characters"));
  }
  for (int i=0; i<l; i++)
  {
    char c = bits.charAt(i);
    int x = bitRead(val, 7-i);
    switch (c) 
    {
      case '0':
        if (x==1)
        {
           bitClear(val, 7-i);
           result = true;
        }
        break;
      case '1':
        if (x==0)
        {
           bitSet(val, 7-i);
           result = true;
        }
        break;
      case '*':
        break;
      default:
        Msg::print(Msg::E, F("changebits: "));
        Msg::print(Msg::E, bits);
        Msg::print(Msg::E, F(" contains invalid character "));
        Msg::println(Msg::E, c);
        return false;
    }
  }
  return result;
}

bool ES9028::_writeRegisterBits(byte regAddr, String bits) 
{
  byte regVal;
  bool ok = _readRegister(regAddr, regVal);
  if (!ok)
    return false;
  ok = _changeByte(regVal, bits);
  if (!ok)
  {
    // nothing to change
    Msg::println(Msg::D, F("-Write value same as register value- "));
    return true;
  }
  return _writeRegister(regAddr, regVal);
}

bool ES9028::reset()
{
  if (getInitialised())
  {
    _writeRegisterBits(0,"*******1");
    _setInitialised(false);
  }
  return true;
}

boolean ES9028::initialise()
{
  _printDAC();
  Msg::println(Msg::I, F("initialising"));
  _initialised = true;
  if (_getChipType(_chipType))  
  {
    _setInitialised(true);
    return setMode(_mode);
  }
  else
  {
    _initialised = false;
    _printDAC(Msg::E);
    Msg::println(Msg::E, F("Initialisation failed"));
    return false;
  }
}

bool ES9028::getInitialised()
{
  return _initialised;
}

void ES9028::_setInitialised(boolean val)
{
  _initialised = val;
  _printDAC(Msg::I);
  if (_initialised)
    Msg::println(Msg::I, F("initialised"));
  else
    Msg::println(Msg::I, F("uninitialised"));
}

ES9028::Mode ES9028::getMode()
{
  return _mode;
}

byte ES9028::getAddress()
{
  return _address;
}

bool ES9028::setOscillatorDrive(OscillatorDrive val)
{
  _printDAC();
  Msg::println(F(": setting Oscillator Drive"));
  switch(val)
  {
  case OSC_ShutDown:
    // Reg 0: shut down the oscillator
    return _writeRegisterBits(0, F("1111****"));
    break;
  case OSC_QuarterBias:
    // Reg 0: � bias
    return _writeRegisterBits(0, F("1110****"));
    break;
  case OSC_HalfBias:
    // Reg 0: � bias
    return _writeRegisterBits(0, F("1100****"));
    break;
  case OSC_ThreeQuarterBias:
    // Reg 0: � bias
    return _writeRegisterBits(0, F("1000****"));
    break;
  case OSC_FullBias:
    // Reg 0: full bias (default)
    return _writeRegisterBits(0, F("0000****"));
    break;
  default:
    return _invalidSetting();
  }
}

bool ES9028::setClockGear(ClockGear val)
{
  _printDAC();
  Msg::println(F(": setting ClockGear"));
  switch(val)
  {
  case XIN:
    // Reg 0: XIN (default)
    return _writeRegisterBits(0, F("****00**"));
    break;
  case XIN_Div2:
    // Reg 0: XIN / 2
    return _writeRegisterBits(0, F("****01**"));
    break;
  case XIN_Div4:
    // Reg 0: XIN / 4
    return _writeRegisterBits(0, F("****10**"));
    break;
  case XIN_Div8:
    // Reg 0: XIN / 8
    return _writeRegisterBits(0, F("****11**"));
    break;
  default:
    return _invalidSetting();
  }
}

bool ES9028::setSPDIFUserBits(SPDIFUserBits val)
{
  _printDAC();
  Msg::println(F("setting SPDIFUserBits"));
  switch(val)
  {
  case SPDIF_PresentUserBits:
    // Reg 1: presents the SPDIF user bits on the read-only register interface
    return _writeRegisterBits(1, F("1*******"));
    break;
  case SPDIF_PresentChannelBits:
    // Reg 1: presents the SPDIF channel status bits on the read-only register interface (default)
    return _writeRegisterBits(1, F("0*******"));
    break;
  default:
    return _invalidSetting();
  }
}

bool ES9028::setSPDIFDataFlag(SPDIFDataFlag val)
{
  _printDAC();
  Msg::println(F("setting SPDIFDataFlag"));
  switch(val)
  {
  case SPDIF_IgnoreDataFlag:
    // Reg 1: ignore the data flag in the channel status bits and continue to process the decoded SPDIF data
    return _writeRegisterBits(1, F("*1******"));
    break;
  case SPDIF_MuteIfDataFlagSet:
    // Reg 1: mute the SPDIF data when the valid flag is invalid (default)
    return _writeRegisterBits(1, F("*0******"));
    break;
  default:
    return _invalidSetting();
  }
}

bool ES9028::setSPDIFValidFlag(SPDIFValidFlag val)
{
  _printDAC();
  Msg::println(F("setting SPDIFValidFlag"));
  switch(val)
  {
  case SPDIF_IgnoreValidFlag:
    // Reg 1: ignore the data flag in the channel status bits and continue to process the decoded SPDIF data
    return _writeRegisterBits(1, F("**1*****"));
    break;
  case SPDIF_MuteIfInvalidValidFlag:
    // Reg 1: mute the SPDIF data when the valid flag is invalid (default)
    return _writeRegisterBits(1, F("**0*****"));
    break;
  default:
    return _invalidSetting();
  }
}

bool ES9028::setAutoSelect(AutoSelect val)
{
  _printDAC();
  Msg::print(F("setting AutoSelect to "));
  switch(val)
  {
  case AutoSelect_DSD_SPDIF_SERIAL:
    // Reg 1: automatically select between DSD, SPDIF or serial data (default)
    Msg::println(F("DSD/SPDIF/SERIAL"));
    return _writeRegisterBits(1, F("****11**"));
    break;
  case AutoSelect_SPDIF_SERIAL:
    // Reg 1: automatically select between SPDIF or serial data
    Msg::println(F("SPDIF/SERIAL"));
    return _writeRegisterBits(1, F("****10**"));
    break;
  case AutoSelect_DSD_SERIAL:
    // Reg 1: automatically select between DSD or serial data
    Msg::println(F("DSD/SERIAL"));
    return _writeRegisterBits(1, F("****01**"));
    break;
  case AutoSelect_Disable:
    // Reg 1: disable automatic input decoder and instead use the information
    Msg::println(F("Disable"));
    return _writeRegisterBits(1, F("****00**"));
    break;
  default:
    return _invalidSetting();
  }
}

bool ES9028::setInputSelect(InputSelect val)
{
  _printDAC();
  Msg::print(F("setting Input Select to "));
  switch(val)
  {
  case InputSelect_DSD:
    Msg::println(F("DSD"));
    return _writeRegisterBits(1, F("******11"));
    break;
  case InputSelect_SPDIF:
    Msg::println(F("SPDIF"));
    return _writeRegisterBits(1, F("******01"));
    break;
  case InputSelect_SERIAL:
    Msg::println(F("SERIAL (default)"));
    return _writeRegisterBits(1, F("******00"));
    break;
  default:
    return _invalidSetting();
  }
}

bool ES9028::setAutoMute(AutoMute val)
{
  _printDAC();
  Msg::print(F("setting AutoMute to "));
  switch(val)
  {
  case AutoMute_MuteAndRampToGnd:
    // Reg 2: perform a mute and then ramp all channels to ground
    Msg::println(F("MuteAndRampToGnd"));
    return _writeRegisterBits(2, F("11******"));
    break;
  case AutoMute_RampToGnd:
    // Reg 2: ramp all channels to ground
    Msg::println(F("RampToGnd"));
    return _writeRegisterBits(2, F("10******"));
    break;
  case AutoMute_Mute:
    // Reg 2: perform a mute
    Msg::println(F("Mute"));
    return _writeRegisterBits(2, F("01******"));
    break;
  case AutoMute_None:
    // Reg 2: normal operation (default)
    Msg::println(F("SNone"));
    return _writeRegisterBits(2, F("00******"));
    break;
  default:
    return _invalidSetting();
  }
}

bool ES9028::setSerialBits(Bits val)
{
  _printDAC();
  Msg::print(F("setting Serial Bits to "));
  switch(val)
  {
  case Bits_Default:
    Msg::println(F("32-bits (default)"));
    return _writeRegisterBits(2, F("**11****"));
    break;
  case Bits_32:
    Msg::println(F("32-bits"));
    return _writeRegisterBits(2, F("**10****"));
    break;
  case Bits_24:
    Msg::println(F("24-bits"));
    return _writeRegisterBits(2, F("**01****"));
    break;
  case Bits_16:
    Msg::println(F("16-bits"));
    return _writeRegisterBits(2, F("**00****"));
    break;
  default:
    return _invalidSetting();
  }
}

bool ES9028::setSerialLength(Bits val)
{
  _printDAC();
  Msg::println(F("setting Serial Length to "));
  switch(val)
  {
  case Bits_Default:
    Msg::println(F("32-bits (default)"));
    return _writeRegisterBits(2, F("****11**"));
    break;
  case Bits_32:
    Msg::println(F("32-bits"));
    return _writeRegisterBits(2, F("****10**"));
    break;
  case Bits_24:
    Msg::println(F("24-bits"));
    return _writeRegisterBits(2, F("****01**"));
    break;
  case Bits_16:
    Msg::println(F("16-bits"));
    return _writeRegisterBits(2, F("****00s**"));
    break;
  default:
    return _invalidSetting();
  }
}

bool ES9028::setSerialMode(SerialMode val)
{
  _printDAC();
  Msg::println(F("setting SerialMode"));
  switch(val)
  {
  case RightJustified:
    // Reg 2: right-justified mode
    return _writeRegisterBits(2, F("******11"));
    break;
  case LeftJustified:
    // Reg 2: left-justified mode
    return _writeRegisterBits(2, F("******01"));
    break;
  case I2S_Mode:
    // Reg 2: I2S mode (default)
    return _writeRegisterBits(2, F("******00"));
    break;
  default:
    return _invalidSetting();
  }
}

bool ES9028::setAutomuteTime(byte val)
{
  _printDAC();
  Msg::println(F("setting AutomuteTime"));
  // Reg 4: Configures the amount of time the audio data must remain below the automute_level before an automute condition is flagged. Defaults to 0 which disables automute
  return _writeRegister(4, val);
}

bool ES9028::setAutomuteLevel(byte val)
{
  _printDAC();
  Msg::println(F("setting AutomuteLevel"));
  // Reg 5: Configures the threshold which the audio must be below before an automute condition is flagged. The level is measured in decibels (dB) and defaults to -104dB.
  return _writeRegister(5, val);
}

bool ES9028::enableAutoDeEmph() // Automatically engages the de-emphasis filters when SPDIF data is provides and the SPDIF channel status bits contains valid de-emphasis settings
{
  _printDAC();
  Msg::println(F("enable AutoDeEmph"));
  return _writeRegisterBits(6, F("1*******"));
}

bool ES9028::disableAutoDeEmph() // disables AutoDeEmph
{
  _printDAC();
  Msg::println(F("disable AutoDeEmph"));
  return _writeRegisterBits(6, F("0*******"));
}

bool ES9028::enableDeEmph() // enables the built-in de-emphasis filters.
{
  _printDAC();
  Msg::println(F("enable AutoDeEmph"));
  return _writeRegisterBits(6, F("1*******"));
}

bool ES9028::disableDeEmph() // disables the built-in de-emphasis filters.
{
  _printDAC();
  Msg::println(F("enable AutoDeEmph"));
  return _writeRegisterBits(6, F("1*******"));
}

bool ES9028::setDeEmphSelect(DeEmphSelect val) // Selects which de-emphasis filter is used.
{
  _printDAC();
  Msg::println(F("setting DeEmphSelect"));
  switch(val)
  {
  case DeEmpESelect_48khz:
    // Reg 6: 48kHz
    return _writeRegisterBits(6, F("**10****"));
    break;
  case DeEmpESelect_441khz:
    // Reg 6: 44.1kHz
    return _writeRegisterBits(6, F("**01****"));
    break;
  case DeEmpESelect_32khz:
    // Reg 6: 32kHz (default)
    return _writeRegisterBits(6, F("**00****"));
    break;
  default:
    return _invalidSetting();
  }
}

bool ES9028::setVolumeRate(byte val)  // Selects a volume ramp rate to use when transitioning between different volume levels. The volume ramp rate is measured in decibels per second (dB/s). Volume rate is in the range 0-7.
{
  _printDAC();
  Msg::println(F("setting Volume Rate"));
  if (val > 7)
    return _invalidSetting();
  byte b;
  if (_readRegister(6, b))
    return _writeRegister(6, b | val);
  return false;
}

bool ES9028::setFilterShape(FilterShape val)  // Selects the type of filter to use during the 8x FIR interpolation phase.
{
  _printDAC();
  Msg::print(F("setting Filter Shape to "));
  switch(val)
  {
  case Filter_Brickwall:
    // brickwall filter
    Msg::println(F("Brickwall"));
    return _writeRegisterBits(7, F("110*****"));
    break;
  case Filter_Hybrid:
    // hybrid, fast roll-off, minimum phase filter
    Msg::println(F("Hybrid"));
    return _writeRegisterBits(7, F("101*****"));
    break;
  case Filter_Apodizing:
    // apodizing, fast roll-off, linear phase filter
    Msg::println(F("Apodizing"));
    return _writeRegisterBits(7, F("100*****"));
    break;
  case Filter_SlowMinPhase:
    // slow roll-off, minimum phase filter
    Msg::println(F("SlowMinPhase"));
    return _writeRegisterBits(7, F("011*****"));
    break;
  case Filter_FastMinPhase:
    // fast roll-off, minimum phase filter (default)
    Msg::println(F("FastMinPhase"));
    return _writeRegisterBits(7, F("010*****"));
    break;
  case Filter_SlowLinPhase:
    // slow roll-off, linear phase filter
    Msg::println(F("SlowLinPhase"));
    return _writeRegisterBits(7, F("001*****"));
    break;
  case Filter_FastLinPhase:
    // fast roll-off, linear phase filter
    Msg::println(F("FastLinPhase"));
    return _writeRegisterBits(7, F("000*****"));
    break;
  default:
    return _invalidSetting();
  }
}

bool ES9028::setIIR_Bandwidth(IIR_Bandwidth val) // Selects the type of filter to use during the 8x IIR interpolation phase
{
  _printDAC();
  Msg::println(F("setting IIR_Bandwidth"));
  switch(val)
  {
  case IIR_70k:
    // 1.5873fs (70k @ 44.1kHz)
    return _writeRegisterBits(7, F("*****11*"));
    break;
  case IIR_60k:
    // 1.3605fs (60k @ 44.1kHz)
    return _writeRegisterBits(7, F("*****10*"));
    break;
  case IIR_50k:
    // 1.1338fs (50k @ 44.1kHz)
    return _writeRegisterBits(7, F("*****01*"));
    break;
  case IIR_4744k:
    // 1.0757fs (IIR_4744k @ 44.1kHz) (default)
    return _writeRegisterBits(7, F("*****00*"));
    break;
  default:
    return _invalidSetting();
  }
}

bool ES9028::mute() // mutes all 8 channels of the SABRE DAC.
{
  _printDAC();
  Msg::println(F("mute"));
  return _writeRegisterBits(7, F("*******1"));
}

bool ES9028::unmute() // unmutes all 8 channels of the SABRE DAC.
{
  _printDAC();
  Msg::println(F("unmute"));
  return _writeRegisterBits(7, F("*******0"));
}

bool ES9028::setGPIO2(GPIO val)
{
  _printDAC();
  Msg::print(F("setting GPIO 2 to "));
  switch(val)
  {
  case GPIO_Automute:
    // Output is high when an automute has been triggered
    Msg::println(F("Automute Status"));
    return _writeRegisterBits(8, F("0000****"));
    break;
  case GPIO_Lock:
    // Reg 8: Output is high when lock is triggered.
    Msg::println(F("Lock Status"));
    return _writeRegisterBits(8, F("0001****"));
    break;
  case GPIO_VolumeMin:
    // Reg 8: Output is high when all digital volume controls have been ramped to minus full scale. This can occur, for example, if automute is enabled and set to mute the volume.
    return _writeRegisterBits(8, F("0010****"));
    break;
  case GPIO_CLK:
    // Reg 8: Output is a buffered MCLK signal which can be used to synchronize other devices.
    return _writeRegisterBits(8, F("0011****"));
    break;
  case GPIO_Interrupt:
    // Reg 8: Output is high when the contents of register 64 have been modified (meaning that the lock_status or automute_status register have been changed). Reading register 64 will clear this interrupt.
    return _writeRegisterBits(8, F("0100****"));
    break;
  case GPIO_ADC_CLK:
    // Reg 8: Output is a buffered ADC clock signal. The ADC clock signal is defined by the adc_clk_sel register.
    return _writeRegisterBits(8, F("0101****"));
    break;
  case GPIO_ForceLow:
    // Reg 8: Output is forced low
    return _writeRegisterBits(8, F("0111****"));
    break;
  case GPIO_StandardInput:
    // Reg 8: Places the GPIO into a high impedance state allowing the customer to provide a digital signal and then read that signal back via the I2C register 65.
    Msg::println(F("Standard Input"));
    return _writeRegisterBits(8, F("1000****"));
    break;
  case GPIO_InputSelect:
    // Reg 8: Places the GPIO into a high impedance state and allows the customer to toggle the input selection between two modes using the GPIO.
    return _writeRegisterBits(8, F("1001****"));
    break;
  case GPIO_MuteAll:
    // Reg 8: Places the GPIO into a high impedance state and allows the customer to force a mute condition by applying a logic high signal to the GPIO.
    return _writeRegisterBits(8, F("1010****"));
    break;
  case GPIO_ADC_Input:
    // Reg 8: gpio1_cfg: GPIO1 becomes ADC2 input
    return _writeRegisterBits(8, F("1101****"));
    break;
  case GPIO_SoftStartComplete:
    // Reg 8: Output is high when the DAC output is ramped to ground.
    return _writeRegisterBits(8, F("1110****"));
    break;
  case GPIO_ForceHigh:
    // Reg 8: Output is forced high
    return _writeRegisterBits(8, F("1111****"));
    break;
  default:
    return _invalidSetting();
  }
}

bool ES9028::setGPIO1(GPIO val)
{
  _printDAC();
  Msg::print(F("setting GPIO 1 to "));
  switch(val)
  {
  case GPIO_Automute:
    // Reg 8: Output is high when an automute has been triggered.
    Msg::println(F("Automute Status"));
    return _writeRegisterBits(8, F("****0000"));
    break;
  case GPIO_Lock:
    // Reg 8: Output is high when lock is triggered.
    Msg::println(F("Lock Status"));
    return _writeRegisterBits(8, F("****0001"));
    break;
  case 2:
    // Reg 8: Output is high when all digital volume controls have been ramped to minus full scale. This can occur, for example, if automute is enabled and set to mute the volume.
    return _writeRegisterBits(8, F("****0010"));
    break;
  case 3:
    // Reg 8: Output is a buffered MCLK signal which can be used to synchronize other devices.
    return _writeRegisterBits(8, F("****0011"));
    break;
  case 4:
    // Reg 8: Output is high when the contents of register 64 have been modified (meaning that the lock_status or automute_status register have been changed). Reading register 64 will clear this interrupt.
    return _writeRegisterBits(8, F("****0100"));
    break;
  case 5:
    // Reg 8: Output is a buffered ADC clock signal. The ADC clock signal is defined by the adc_clk_sel register.
    return _writeRegisterBits(8, F("****0101"));
    break;
  case 6:
    // Reg 8: Output is forced low
    return _writeRegisterBits(8, F("****0111"));
    break;
  case GPIO_StandardInput:
    // Reg 8: Places the GPIO into a high impedance state allowing the customer to provide a digital signal and then read that signal back via the I2C register 65.
    Msg::println(F("Standard Input"));
    return _writeRegisterBits(8, F("****1000"));
    break;
  case 8:
    // Reg 8: Places the GPIO into a high impedance state and allows the customer to toggle the input selection between two modes using the GPIO.
    return _writeRegisterBits(8, F("****1001"));
    break;
  case 9:
    // Reg 8: Places the GPIO into a high impedance state and allows the customer to force a mute condition by applying a logic high signal to the GPIO.
    return _writeRegisterBits(8, F("****1010"));
    break;
  case 10:
    // Reg 8: gpio1_cfg: GPIO1 becomes ADC2 input
    return _writeRegisterBits(8, F("****1101"));
    break;
  case 11:
    // Reg 8: Output is high when the DAC output is ramped to ground.
    return _writeRegisterBits(8, F("****1110"));
    break;
  case 12:
    // Reg 8: Output is forced high
    return _writeRegisterBits(8, F("****1111"));
    break;
  default:
    return _invalidSetting();
  }
}

bool ES9028::setGPIO4(GPIO val)
{
  _printDAC();
  Msg::print(F("setting GPIO 4 to "));
  switch(val)
  {
  case GPIO_Automute:
    // Reg 9: Output is high when an automute has been triggered.
    Msg::println(F("Automute Status"));
    return _writeRegisterBits(9, F("0000****"));
    break;
  case GPIO_Lock:
    // Reg 9: Output is high when lock is triggered.
    Msg::println(F("Lock Status"));
    return _writeRegisterBits(9, F("0001****"));
    break;
  case 2:
    // Reg 9: Output is high when all digital volume controls have been ramped to minus full scale. This can occur, for example, if automute is enabled and set to mute the volume.
    return _writeRegisterBits(9, F("0010****"));
    break;
  case 3:
    // Reg 9: Output is a buffered MCLK signal which can be used to synchronize other devices.
    return _writeRegisterBits(9, F("0011****"));
    break;
  case 4:
    // Reg 9: Output is high when the contents of register 64 have been modified (meaning that the lock_status or automute_status register have been changed). Reading register 64 will clear this interrupt.
    return _writeRegisterBits(9, F("0100****"));
    break;
  case 5:
    // Reg 9: Output is a buffered ADC clock signal. The ADC clock signal is defined by the adc_clk_sel register.
    return _writeRegisterBits(9, F("0101****"));
    break;
  case 6:
    // Reg 9: Output is forced low
    return _writeRegisterBits(9, F("0111****"));
    break;
  case GPIO_StandardInput:
    // Reg 9: Places the GPIO into a high impedance state allowing the customer to provide a digital signal and then read that signal back via the I2C register 65.
    Msg::println(F("Standard Input"));
    return _writeRegisterBits(9, F("1000****"));
    break;
  case 8:
    // Reg 9: Places the GPIO into a high impedance state and allows the customer to toggle the input selection between two modes using the GPIO.
    return _writeRegisterBits(9, F("1001****"));
    break;
  case 9:
    // Reg 9: Places the GPIO into a high impedance state and allows the customer to force a mute condition by applying a logic high signal to the GPIO.
    return _writeRegisterBits(9, F("1010****"));
    break;
  case 10:
    // Reg 9: gpio1_cfg: GPIO1 becomes ADC2 input
    return _writeRegisterBits(9, F("1101****"));
    break;
  case 11:
    // Reg 9: Output is high when the DAC output is ramped to ground.
    return _writeRegisterBits(9, F("1110****"));
    break;
  case 12:
    // Reg 9: Output is forced high
    return _writeRegisterBits(9, F("1111****"));
    break;
  default:
    return _invalidSetting();
  }
}

bool ES9028::setGPIO3(GPIO val)
{
  _printDAC();
  Msg::print(F("setting GPIO 3 to "));
  switch(val)
  {
  case GPIO_Automute:
    // Reg 9: Output is high when an automute has been triggered.
    Msg::println(F("Automute Status"));
    return _writeRegisterBits(9, F("****0000"));
    break;
  case GPIO_Lock:
    // Reg 9: Output is high when lock is triggered.
    Msg::println(F("Lock Status"));
    return _writeRegisterBits(9, F("****0001"));
    break;
  case 2:
    // Reg 9: Output is high when all digital volume controls have been ramped to minus full scale. This can occur, for example, if automute is enabled and set to mute the volume.
    return _writeRegisterBits(9, F("****0010"));
    break;
  case 3:
    // Reg 9: Output is a buffered MCLK signal which can be used to synchronize other devices.
    return _writeRegisterBits(9, F("****0011"));
    break;
  case 4:
    // Reg 9: Output is high when the contents of register 64 have been modified (meaning that the lock_status or automute_status register have been changed). Reading register 64 will clear this interrupt.
    return _writeRegisterBits(9, F("****0100"));
    break;
  case 5:
    // Reg 9: Output is a buffered ADC clock signal. The ADC clock signal is defined by the adc_clk_sel register.
    return _writeRegisterBits(9, F("****0101"));
    break;
  case 6:
    // Reg 9: Output is forced low
    return _writeRegisterBits(9, F("****0111"));
    break;
  case GPIO_StandardInput:
    // Reg 9: Places the GPIO into a high impedance state allowing the customer to provide a digital signal and then read that signal back via the I2C register 65.
    Msg::println(F("Standard Input"));
    return _writeRegisterBits(9, F("****1000"));
    break;
  case 8:
    // Reg 9: Places the GPIO into a high impedance state and allows the customer to toggle the input selection between two modes using the GPIO.
    return _writeRegisterBits(9, F("****1001"));
    break;
  case 9:
    // Reg 9: Places the GPIO into a high impedance state and allows the customer to force a mute condition by applying a logic high signal to the GPIO.
    return _writeRegisterBits(9, F("****1010"));
    break;
  case 10:
    // Reg 9: gpio1_cfg: GPIO1 becomes ADC2 input
    return _writeRegisterBits(9, F("****1101"));
    break;
  case 11:
    // Reg 9: Output is high when the DAC output is ramped to ground.
    return _writeRegisterBits(9, F("****1110"));
    break;
  case 12:
    // Reg 9: Output is forced high
    return _writeRegisterBits(9, F("****1111"));
    break;
  default:
    return _invalidSetting();
  }
}

bool ES9028::enableMasterMode()
{
  _printDAC();
  Msg::println(F("enable MasterMode"));
  return _writeRegisterBits(10, F("1*******"));
}

bool ES9028::disableMasterMode()
{
  _printDAC();
  Msg::println(F("disableMasterMode"));
  return _writeRegisterBits(10, F("0*******"));
}

bool ES9028::setMasterDiv(MasterDiv val)
{
  _printDAC();
  Msg::println(F("setting MasterDiv"));
  switch(val)
  {
  case 0:
    // Reg 10: DATA_CLK frequency = MCLK/2 (default)
    return _writeRegisterBits(10, F("*00*****"));
    break;
  case 1:
    // Reg 10: DATA_CLK frequency = MCLK/4 
    return _writeRegisterBits(10, F("*01*****"));
    break;
  case 2:
    // Reg 10: DATA_CLK frequency = MCLK/8 
    return _writeRegisterBits(10, F("*10*****"));
    break;
  case 3:
    // Reg 10: DATA_CLK frequency = MCLK/16 
    return _writeRegisterBits(10, F("*11*****"));
    break;
  default:
    return _invalidSetting();
  }
}

bool ES9028::enable128fsMode()
{
  _printDAC();
  Msg::println(F("enable enable128fsMode"));
  return _writeRegisterBits(10, F("***1****"));
}

bool ES9028::disable128fsMode()
{
  _printDAC();
  Msg::println(F("enable disable128fsMode"));
  return _writeRegisterBits(10, F("***0****"));
}

bool ES9028::setLockSpeed(LockSpeed val)
{
  _printDAC();
  Msg::println(F("setting LockSpeed"));
  switch(val)
  {
  case 0:
    // Reg 10: 16384 FSL edges (default)
    return _writeRegisterBits(10, F("****0000"));
    break;
  case 1:
    // Reg 10: 8192 FSL edges 
    return _writeRegisterBits(10, F("****0001"));
    break;
  case 2:
    // Reg 10: 5461 FSL edges 
    return _writeRegisterBits(10, F("****0010"));
    break;
  case 3:
    // Reg 10: 4096 FSL edges 
    return _writeRegisterBits(10, F("****0011"));
    break;
  case 4:
    // Reg 10: 3276 FSL edges 
    return _writeRegisterBits(10, F("****0100"));
    break;
  case 5:
    // Reg 10: 2730 FSL edges 
    return _writeRegisterBits(10, F("****0101"));
    break;
  case 6:
    // Reg 10: 2340 FSL edges 
    return _writeRegisterBits(10, F("****0110"));
    break;
  case 7:
    // Reg 10: 2048 FSL edges 
    return _writeRegisterBits(10, F("****0111"));
    break;
  case 8:
    // Reg 10: 1820 FSL edges
    return _writeRegisterBits(10, F("****1001"));
    break;
  case 9:
    // Reg 10: 1638 FSL edges
    return _writeRegisterBits(10, F("****1001"));
    break;
  case 10:
    // Reg 10: 1489 FSL edges 
    return _writeRegisterBits(10, F("****1010"));
    break;
  case 11:
    // Reg 10: 1365 FSL edges 
    return _writeRegisterBits(10, F("****1011"));
    break;
  case 12:
    // Reg 10: 1260 FSL edges
    return _writeRegisterBits(10, F("****1100"));
    break;
  case 13:
    // Reg 10: 1170 FSL edges 
    return _writeRegisterBits(10, F("****1101"));
    break;
  case 14:
    // Reg 10: 1092 FSL edges 
    return _writeRegisterBits(10, F("****1110"));
    break;
  case 15:
    // Reg 10: 1024 FSL edges 
    return _writeRegisterBits(10, F("****1111"));
    break;
  default:
    return _invalidSetting();
  }
}

bool ES9028::setSPDIFInput(SPDIFInput val)
{
  _printDAC();
  Msg::println(F("setting SPDIFInput"));
  switch(val)
  {
  case 0:
    // Reg 11: DATA_CLK (default)
    return _writeRegisterBits(11, F("0000****"));
    break;
  case 1:
    // Reg 11: DATA1
    return _writeRegisterBits(11, F("0001****"));
    break;
  case 2:
    // Reg 11: DATA2
    return _writeRegisterBits(11, F("0010****"));
    break;
  case 3:
    // Reg 11: DATA3
    return _writeRegisterBits(11, F("0011****"));
    break;
  case 4:
    // Reg 11: DATA4
    return _writeRegisterBits(11, F("0100****"));
    break;
  case 5:
    // Reg 11: DATA5
    return _writeRegisterBits(11, F("0101****"));
    break;
  case 6:
    // Reg 11: DATA6
    return _writeRegisterBits(11, F("0110****"));
    break;
  case 7:
    // Reg 11: DATA7
    return _writeRegisterBits(11, F("0111****"));
    break;
  case 8:
    // Reg 11: DATA8
    return _writeRegisterBits(11, F("1001****"));
    break;
  case 9:
    // Reg 11: GPIO1
    return _writeRegisterBits(11, F("1001****"));
    break;
  case 10:
    // Reg 11: GPIO2
    return _writeRegisterBits(11, F("1010****"));
    break;
  case 11:
    // Reg 11: GPIO3
    return _writeRegisterBits(11, F("1011****"));
    break;
  case 12:
    // Reg 11: GPIO4
    return _writeRegisterBits(11, F("1100****"));
    break;
  default:
    return _invalidSetting();
  }
}

bool ES9028::invertGPIO1() // Inverts the GPIO output when set.
{
  _printDAC();
  Msg::println(F("invert GPIO 1"));
  return _writeRegisterBits(11, F("*******1"));
}

bool ES9028::invertGPIO2() // Inverts the GPIO output when set.
{
  _printDAC();
  Msg::println(F("invert GPIO 2"));
  return _writeRegisterBits(11, F("******1*"));
}

bool ES9028::invertGPIO3() // Inverts the GPIO output when set.
{
  _printDAC();
  Msg::println(F("invert GPIO 3"));
  return _writeRegisterBits(11, F("*****1**"));
}

bool ES9028::invertGPIO4() // non-inverts the GPIO output when set.
{
  _printDAC();
  Msg::println(F("non-invert GPIO 4"));
  return _writeRegisterBits(11, F("****1***"));
}

bool ES9028::nonInvertGPIO1() // non-inverts the GPIO output when set.
{
  _printDAC();
  Msg::println(F("non-invert GPIO 1"));
  return _writeRegisterBits(11, F("*******0"));
}

bool ES9028::nonInvertGPIO2() // non-inverts the GPIO output when set.
{
  _printDAC();
  Msg::println(F("non-invert GPIO 2"));
  return _writeRegisterBits(11, F("******0*"));
}

bool ES9028::nonInvertGPIO3() // Inverts the GPIO output when set.
{
  _printDAC();
  Msg::println(F("non-invert GPIO 3"));
  return _writeRegisterBits(11, F("*****0**"));
}

bool ES9028::nonInvertGPIO4() // non-inverts the GPIO output when set.
{
  _printDAC();
  Msg::println(F("non-invert GPIO 4"));
  return _writeRegisterBits(11, F("****0***"));
}

bool ES9028::setDpllBandwidthSerial(DpllBandwidth val)
{
  _printDAC();
  Msg::println(F("setting serial DpllBandwidth"));
  switch(val)
  {
  case 0:
    // Reg 12: DPLL Off
    return _writeRegisterBits(12, F("0000****"));
    break;
  case 1:
    // Reg 12: Lowest Bandwidth
    return _writeRegisterBits(12, F("0001****"));
    break;
  case 2:
    return _writeRegisterBits(12, F("0010****"));
    break;
  case 3:
    return _writeRegisterBits(12, F("0011****"));
    break;
  case 4:
    return _writeRegisterBits(12, F("0100****"));
    break;
  case 5:
    // Reg 12: (default)
    return _writeRegisterBits(12, F("0101****"));
    break;
  case 6:
    return _writeRegisterBits(12, F("0110****"));
    break;
  case 7:
    return _writeRegisterBits(12, F("0111****"));
    break;
  case 8:
    return _writeRegisterBits(12, F("1000****"));
    break;
  case 9:
    return _writeRegisterBits(12, F("1001****"));
    break;
  case 10:
    return _writeRegisterBits(12, F("1010****"));
    break;
  case 11:
    return _writeRegisterBits(12, F("1011****"));
    break;
  case 12:
    return _writeRegisterBits(12, F("1100****"));
    break;
  case 13:
    return _writeRegisterBits(12, F("1101****"));
    break;
  case 14:
    return _writeRegisterBits(12, F("1110****"));
    break;
  case 15:
    return _writeRegisterBits(12, F("1111****"));
    break;
  default:
    return _invalidSetting();
  }
}

bool ES9028::setDpllBandwidthDSD(DpllBandwidth val)
{
  _printDAC();
  Msg::println(F("setting DSD DpllBandwidth"));
  switch(val)
  {
  case 0:
    // Reg 12: DPLL Off
    return _writeRegisterBits(12, F("****0000"));
    break;
  case 1:
    // Reg 12: Lowest Bandwidth
    return _writeRegisterBits(12, F("****0001"));
    break;
  case 2:
    return _writeRegisterBits(12, F("****0010"));
    break;
  case 3:
    return _writeRegisterBits(12, F("****0011"));
    break;
  case 4:
    return _writeRegisterBits(12, F("****0100"));
    break;
  case 5:
    // Reg 12: (default)
    return _writeRegisterBits(12, F("****0101"));
    break;
  case 6:
    return _writeRegisterBits(12, F("****0110"));
    break;
  case 7:
    return _writeRegisterBits(12, F("****0111"));
    break;
  case 8:
    return _writeRegisterBits(12, F("****1000"));
    break;
  case 9:
    return _writeRegisterBits(12, F("****1001"));
    break;
  case 10:
    return _writeRegisterBits(12, F("****1010"));
    break;
  case 11:
    return _writeRegisterBits(12, F("****1011"));
    break;
  case 12:
    return _writeRegisterBits(12, F("****1100"));
    break;
  case 13:
    return _writeRegisterBits(12, F("****1101"));
    break;
  case 14:
    return _writeRegisterBits(12, F("****1110"));
    break;
  case 15:
    return _writeRegisterBits(12, F("****1111"));
    break;
  default:
    return _invalidSetting();
  }
}

bool ES9028::enableJitterEliminator() 
{
  _printDAC();
  Msg::println(F("enable Jitter Eliminator"));
  return _writeRegisterBits(13, F("**1*****"));
}

bool ES9028::disableJitterEliminator() 
{
  _printDAC();
  Msg::println(F("disable Jitter Eliminator"));
  return _writeRegisterBits(13, F("**0*****"));
}

bool ES9028::enableTHDcompensation() 
{
  _printDAC();
  Msg::println(F("enable THD compensation"));
  return _writeRegisterBits(13, F("*0******"));
}

bool ES9028::disableTHDcompensation() 
{
  _printDAC();
  Msg::println(F("disable THD compensation"));
  return _writeRegisterBits(13, F("*1******"));
}

bool ES9028::enableDither() 
{
  _printDAC();
  Msg::println(F("enable Dither"));
  return _writeRegisterBits(13, F("0*******"));
}

bool ES9028::disableDither() 
{
  _printDAC();
  Msg::println(F("disable Dither"));
  return _writeRegisterBits(13, F("1*******"));
}

bool ES9028::setSoftStart(SoftStart val)
{
  _printDAC();
  Msg::println(F("setting Soft Start"));
  switch(val)
  {
  case 0:
    // Reg 14: ramps the output stream to ground
    return _writeRegisterBits(14, F("1*******"));
    break;
  case 1:
    // Reg 14: normal operation (default) will ramp the output stream to AVCC/2 
    return _writeRegisterBits(14, F("0*******"));
    break;
  default:
    return _invalidSetting();
  }
}

bool ES9028::enableSoftStopOnUnlock() 
{
  _printDAC();
  Msg::println(F("enable Soft Stop On Unlock"));
  return _writeRegisterBits(14, F("*1******"));
}

bool ES9028::disableSoftStopOnUnlock() 
{
  _printDAC();
  Msg::println(F("disable Soft Stop On Unlock"));
  return _writeRegisterBits(14, F("*0******"));
}

bool ES9028::setSoftStartTime(byte val) 
{
  _printDAC();
  Msg::println(F("set Soft Start Time"));
  if (val > 20)
    return _invalidSetting();
  else
  { 
    byte reg14;
    if (_readRegister(14, reg14))
    {
      // clear bits 4:0
      reg14 = reg14 & B11100000;
      return (_writeRegister(14, reg14 | val));
    }
    else
      return false;
  }
}

bool ES9028::setGPIOSelect2(InputSelect val)  
// Selects which input type will be selected when GPIOX = 1�b1
{
  _printDAC();
  Msg::println(F("setting GPIO Select 2"));
  switch(val)
  {
  case InputSelect_DSD:
    return _writeRegisterBits(15, F("11******"));
    break;
  case InputSelect_SPDIF:
    return _writeRegisterBits(15, F("01******"));
    break;
  case InputSelect_SERIAL:
    return _writeRegisterBits(15, F("00******"));
    break;
  default:
    return _invalidSetting();
  }
}

bool ES9028::setGPIOSelect1(InputSelect val)  
// Selects which input type will be selected when GPIOX = 1�b1
{
  _printDAC();
  Msg::println(F("setting GPIO Select 1"));
  switch(val)
  {
  case InputSelect_DSD:
    return _writeRegisterBits(15, F("**11****"));
    break;
  case InputSelect_SPDIF:
    return _writeRegisterBits(15, F("**01****"));
    break;
  case InputSelect_SERIAL:
    return _writeRegisterBits(15, F("**00****"));
    break;
  default:
    return _invalidSetting();
  }
}

bool ES9028::setMode(Mode val)  
// Selects which input type will be selected when GPIOX = 1�b1
{
  _printDAC();
  Msg::print(F("setting Mode to "));
  switch(val)
  {
  case MonoLeft:
    Msg::println(F("Mono Left"));
    return mapInputs(Input_1, Input_1, Input_1, Input_1, Input_1, Input_1, Input_1, Input_1);
  case MonoRight:
    Msg::println(F("Mono Right"));
    return mapInputs(Input_2, Input_2, Input_2, Input_2, Input_2, Input_2, Input_2, Input_2);
  case Stereo:
    Msg::println(F("Stereo"));
    return _writeRegisterBits(15, F("*****1**"));
  case EightChannel:
    Msg::println(F("8 Channel"));
    return mapInputs(Input_1, Input_2, Input_3, Input_4, Input_5, Input_6, Input_7, Input_8);
    break;
  case DualLeft:
    Msg::println(F("Dual Left"));
    return mapInputs(Input_1, Input_3, Input_1, Input_3, Input_1, Input_3, Input_1, Input_3);
  case DualRight:
    Msg::println(F("Dual Right"));
    return mapInputs(Input_2, Input_4, Input_2, Input_4, Input_2, Input_4, Input_2, Input_4);
  default:
    return _invalidSetting();
  }
}

bool ES9028::setAttenuation(byte attenuation) // sets the same attenuation for each DAC
{
  if (setVolumeMode(Volume_UseChannel1))
    return setVolume1(attenuation);
  else
    return false;
}

bool ES9028::setVolumeMode(VolumeMode val)  
// Force all eight channels to use the volume coefficients from channel 1.
{
  _printDAC();
  Msg::print(F("setting Volume Mode to "));
  switch(val)
  {
  case Volume_Independent:
    Msg::println(F("Independent"));
    return _writeRegisterBits(15, F("******0*"));
    break;
  case Volume_UseChannel1:
    Msg::println(F("Use Channel 1"));
    return _writeRegisterBits(15, F("******1*"));
    break;
  default:
    return _invalidSetting();
  }
}

bool ES9028::enableVolumeLatching() 
{
  _printDAC();
  Msg::println(F("enable Volume Latching"));
  return _writeRegisterBits(15, F("*******1"));
}

bool ES9028::disableVolumeLatching() 
{
  _printDAC();
  Msg::println(F("disable Volume Latching"));
  return _writeRegisterBits(15, F("*******0"));
}

bool ES9028::setVolume1(byte val) 
{
  _printDAC();
  Msg::println(F("set Volume 1"));
  return (_writeRegister(16, val));
}

bool ES9028::setVolume2(byte val) 
{
  _printDAC();
  Msg::println(F("set Volume 2"));
  return (_writeRegister(17, val));
}

bool ES9028::setVolume3(byte val) 
{
  _printDAC();
  Msg::println(F("set Volume 3"));
  return (_writeRegister(18, val));
}

bool ES9028::setVolume4(byte val) 
{
  _printDAC();
  Msg::println(F("set Volume 4"));
  return (_writeRegister(19, val));
}

bool ES9028::setVolume5(byte val) 
{
  _printDAC();
  Msg::println(F("set Volume 5"));
  return (_writeRegister(20, val));
}

bool ES9028::setVolume6(byte val) 
{
  _printDAC();
  Msg::println(F("set Volume 6"));
  return (_writeRegister(21, val));
}

bool ES9028::setVolume7(byte val) 
{
  _printDAC();
  Msg::println(F("set Volume 7"));
  return (_writeRegister(22, val));
}

bool ES9028::setVolume8(byte val) 
{
  _printDAC();
  Msg::println(F("set Volume 8"));
  return (_writeRegister(23, val));
}

bool ES9028::setMasterTrim(unsigned long val)
{
  _printDAC();
  Msg::println(F("set Master Trim"));
  byte buf[4];   
  buf[0] = (byte) val;
  buf[1] = (byte) val >> 8;
  buf[2] = (byte) val >> 16;
  buf[3] = (byte) val >> 24;  
  if (_writeRegister(24, buf[0]))
    if (_writeRegister(25, buf[1]))
      if (_writeRegister(26, buf[2]))
        return (_writeRegister(27, buf[3]));
  return false;
}

bool ES9028::setTHDCompensationC2(int val)
{
  _printDAC();
  Msg::println(F("set THD Compensation C2"));
  byte buf[2];   
  buf[0] = (byte) val;
  buf[1] = (byte) val >> 8;
  if (_writeRegister(28, buf[0]))
    return (_writeRegister(29, buf[1]));
  return false;
}

bool ES9028::setTHDCompensationC3(int val)
{
  _printDAC();
  Msg::println(F("set THD Compensation C3"));
  byte buf[2];   
  buf[0] = (byte) val;
  buf[1] = (byte) val >> 8;
  if (_writeRegister(30, buf[0]))
    return (_writeRegister(31, buf[1]));
  return false;
}

bool ES9028::setFIRCoeffStage(FIRCoeffStage val)  // Selects which stage of the filter to write.
{
  _printDAC();
  Msg::println(F("setting FIR Coefficient Stage"));
  switch(val)
  {
  case 0:
    // selects stage 1 of the oversampling filter (default)
    return _writeRegisterBits(32, F("0*******"));
    break;
  case 1:
    // selects stage 2 of the oversampling filter
    return _writeRegisterBits(32, F("1*******"));
    break;
  default:
    return _invalidSetting();
  }
}

bool ES9028::setFIRCoeffAddr(byte val) 
{
  _printDAC();
  Msg::println(F("set FIR Coefficient Address"));
  if (val > 127)
    return _invalidSetting();
  else
  { 
    byte reg32;
    if (_readRegister(32, reg32))
    {
      // clear bits 6:0
      reg32 = reg32 & B10000000;
      return (_writeRegister(32, reg32 | val));
    }
    else
      return false;
  }
}

bool ES9028::setFIRCoeff(unsigned long val)
{
  _printDAC();
  Msg::println(F("set FIR Coefficient"));
  byte buf[4];   
  buf[0] = (byte) val;
  buf[1] = (byte) val >> 8;
  buf[2] = (byte) val >> 16;
  buf[3] = (byte) val >> 24;  
  if (_writeRegister(33, buf[0]))
    if (_writeRegister(34, buf[1]))
      if (_writeRegister(35, buf[2]))
        return (_writeRegister(36, buf[3]));
  return false;
}

bool ES9028::enableFIRExternalBypassOSF() //enables the use of an external 8x upsampling filter, bypassing the internal interpolating FIR filter.
{
  _printDAC();
  Msg::println(F("enable FIR Bypass OSF"));
  return _writeRegisterBits(37, F("1*******"));
}

bool ES9028::disableFIRExternalBypassOSF() //uses the internal interpolating FIR filter.
{
  _printDAC();
  Msg::println(F("disable FIR Bypass OSF"));
  return _writeRegisterBits(37, F("0*******"));
}

bool ES9028::enableFIRExtendedFilterLength() //uses an extended 256-tap first stage filter at the expense of disabling oversampling on channels 3-8. This mode should only be used when in stereo operation and with channel mapping
{
  _printDAC();
  Msg::println(F("enable FIR Extended Filter Length"));
  return _writeRegisterBits(37, F("***1****"));
}

bool ES9028::disableFIRExternalFilterLength() // uses the standard 128-tap first stage filter when in fast rolloff mode (default)
{
  _printDAC();
  Msg::println(F("disable FIR Extended Filter Length"));
  return _writeRegisterBits(37, F("***0****"));
}

bool ES9028::enableFIRProgExt() // prog_coeff_addr maps to coefficients 0-127 (default)
{
  _printDAC();
  Msg::println(F("enable FIR Extended Filter Length"));
  return _writeRegisterBits(37, F("*****1**"));
}

bool ES9028::disableFIRProgExt() // prog_coeff_addr maps to coefficients 128-255
{
  _printDAC();
  Msg::println(F("enable FIR Extended Filter Length"));
  return _writeRegisterBits(37, F("*****0**"));
}

bool ES9028::setFIRStage2(FIRStage2 val)  // Selects the symmetry of the stage 2 oversampling filter.
{
  _printDAC();
  Msg::println(F("setting FIR Stage2 filter"));
  switch(val)
  {
  case 0:
    // Uses a sine symmetric filter (27 coefficients) (default)
    return _writeRegisterBits(37, F("*****0**"));
    break;
  case 1:
    // Uses a cosine symmetric filter (28 coefficients)
    return _writeRegisterBits(37, F("*****1**"));
    break;
  default:
    return _invalidSetting();
  }
}

bool ES9028::enableFIRProgCoeffWrite() // Enables writing to the programmable coefficient RAM.
{
  _printDAC();
  Msg::println(F("enable writing to the programmable coefficient RAM"));
  return _writeRegisterBits(37, F("******1*"));
}

bool ES9028::disableFIRProgCoeffWrite() // Disables writing to the programmable coefficient RAM.
{
  _printDAC();
  Msg::println(F("disable writing to the programmable coefficient RAM"));
  return _writeRegisterBits(37, F("******0*"));
}

bool ES9028::enableFIRProgCoeff() // Uses the coefficients programmed via prog_coeff_data
{
  _printDAC();
  Msg::println(F("enable FIR Programmed Coefficients"));
  return _writeRegisterBits(37, F("*******1"));
}

bool ES9028::disableFIRProgCoeff() // Uses a built-in filter selected by filter_shape (default)
{
  _printDAC();
  Msg::println(F("disable FIR Programmed Coefficients"));
  return _writeRegisterBits(37, F("*******0"));
}

bool ES9028::_setInputs(byte reg, Input inputA, Input inputB)
{
  return _writeRegister(reg, (inputA << 4) | inputB);
}

bool ES9028::mapInputs(Input dac1, Input dac2, Input dac3, Input dac4, Input dac5, Input dac6, Input dac7, Input dac8)
{
  _printDAC();
  Msg::println(F("map inputs"));
  if (_writeRegisterBits(15, F("*****0**")) )
    if (_setInputs(38, dac1, dac2))
      if (_setInputs(39, dac3, dac4))
        if (_setInputs(40, dac5, dac6))
           return _setInputs(41, dac7, dac8);
  return false;
}

bool ES9028::setProgrammableNCO(unsigned long val) // An unsigned 32-bit quantity that provides the ratio between MCLK and DATA_CLK. This value can be used to generate arbitrary DATA_CLK frequencies in master mode.
{
  _printDAC();
  Msg::println(F("set programmable NCO"));
  byte buf[4];   
  buf[0] = (byte) val;
  buf[1] = (byte) val >> 8;
  buf[2] = (byte) val >> 16;
  buf[3] = (byte) val >> 24;  
  if (_writeRegister(42, buf[0]))
    if (_writeRegister(43, buf[1]))
      if (_writeRegister(44, buf[2]))
        return (_writeRegister(45, buf[3]));
  return false;
}

bool ES9028::setChannelGain(Gain dac1, Gain dac2, Gain dac3, Gain dac4, Gain dac5, Gain dac6, Gain dac7, Gain dac8) // Note: The +18dB gain only works in PCM mode and is applied prior to the channel mapping.
{
  _printDAC();
  Msg::println(F("set channel gain"));
  byte b = dac1 | (dac2 << 1) | (dac3 << 2) | (dac4 << 3) | (dac5 << 4) | (dac6 << 5) | (dac7 << 6) | (dac8 << 7);
  return _writeRegister(62, b);
}

bool ES9028::GPIO_Status(bool &gpio1, bool &gpio2, bool &gpio3, bool &gpio4)
{
  _printDAC();
  Msg::println(F("get GPOI status"));
  byte b;
  if (_readRegister(65, b))
  {
    gpio1 = b & B1;
    gpio2 = (b >> 1) & B1;
    gpio3 = (b >> 2) & B1;
    gpio4 = (b >> 3) & B1;
  }
  else
    return false;
}

ES9028::ChipType ES9028::chipType()
{
  return _chipType;
}

bool ES9028::_getChipType(ChipType &chipType)
{
  Msg::println(F("-> reading chip ID"));
  byte r;
  if (_readRegister(64, r))
  {
    r = r >> 2;
    Msg::print(F("-> Chip ID is "));
    Msg::println(String(r, BIN));
    //if ((r == B101001) || (r == B101000)) dimdims code
    if (r == B101000)
      chipType = Chip_ES9028PRO;
    else  if (r == B101001)
      chipType = Chip_ES9028PRO;
    else  if (r == B101010)
      chipType = Chip_ES9038PRO;
    return true;
  }
  chipType = Chip_Unknown;
  return false;
}

bool ES9028::getAutomuted(bool &automuteStatus)
{
  byte b;
  if (_readRegister(64, b))
  {
    automuteStatus = b & B00000010;
    return true;
  }
  else
    return false;
}

bool ES9028::locked()
{
  bool l;
  if (_locked(l))
    return l;
  else
    return false;
}

bool ES9028::locked(bool &readError)
{
  bool l;
  if (_locked(l))
  {
    readError = false;
    return l;
  }
  else
  {
    readError = true;
    return false;
  }
}

bool ES9028::_locked(bool &lockStatus)
{
  byte status;
  if (_readRegister(64, status))
  {
    lockStatus = status & B00000001;
    return true;
  }
  lockStatus = false;
  return false;
}

bool ES9028::dopValid() // returns true if the DoP decoder has detected a valid DoP signal on the I2S or SPDIF inputs.
{
  _printDAC();
  Msg::println(F("read DOP status"));
  byte b;
  if (_readRegister(100, b))
  {
    return b & B00001000;
  }
  else
    return false;
}

bool ES9028::spdifValid()  // returns true if the SPDIF decoder has decoded a sequence of valid SPDIF frames.
{
  _printDAC();
  Msg::println(F("read SPDIF status"));
  byte b;
  if (_readRegister(100, b))
  {
    return b & B00000100;
  }
  else
    return false;
}

bool ES9028::i2sValid()  // returns true if the I2S decoder has detected a valid frame clock and bit clock arrangement.
{
  _printDAC();
  Msg::println(F("read I2S status"));
  byte b;
  if (_readRegister(100, b))
  {
    return b & B00000010;
  }
  else
    return false;
}

bool ES9028::dsdValid()  // returns true if the DSD decoder is being used as a fallback option if I2S and SPDIF have both failed to decode their respective input signals.
{
  _printDAC();
  Msg::println(F("read DSD status"));
  byte b;
  if (_readRegister(100, b))
  {
    return b & B00000001;
  }
  else
    return false;
}

ES9028::SignalType ES9028::getSignalType()  // returns signal type.
{
  _printDAC();
  Msg::print(F("read signal type: "));
  byte b;
  if (_readRegister(100, b))
  {
    if (b & B00000001)
    {
      Msg::println(F("DSD"));
      return ES9028::Signal_DSD;
    }
    if (b & B00000010)
    {
      Msg::println(F("DSD"));
      return ES9028::Signal_I2S;
    }
    if (b & B00000100)
    {
      Msg::println(F("DSD"));
      return ES9028::Signal_SPDIF;
    }
    if (b & B00001000)
    {
      Msg::println(F("DSD"));
      return ES9028::Signal_DoP;
    }
  }
  Msg::println(F("No Signal"));
  return ES9028::Signal_NONE;
}

unsigned long ES9028::dpllNumber()  // returns the ratio between the MCLK and the audio clock rate once the DPLL has acquired lock
{
  _printDAC();
  Msg::println(F("read DPLL Number: "));
  byte b1 = 0;
  byte b2 = 0;
  byte b3 = 0;
  byte b4 = 0;
  long dpllnum = 0;
  _readRegister(66, b1);
  _readRegister(67, b2);
  _readRegister(68, b3);
  _readRegister(69, b4);
  dpllnum += (b1 << 24);
  Msg::println(String(dpllnum));
  return dpllnum;
}

unsigned long ES9028::getSampleRate()  // returns the sample rate
{
  _printDAC();
  Msg::println(F("read sample rate: "));
  long sampleRate = (dpllNumber()*clock)/42940;
  Msg::println(String(sampleRate));
  return sampleRate;
}


