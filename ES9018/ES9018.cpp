#include "ES9018.h"


ES9018::ES9018(String name, Clock value)
{
  _name = name;
  _clock = value;       
}

ES9018::ES9018(String name, Clock value, Mode mode)
{
  _name = name;
  _clock = value;       
  if (mode == MonoRight)
  {
    _address = 0x49; // set default I2C address for mono right config
  }
  else
  {
  }
  _setMode(mode);
}

ES9018::ES9018(String name, Clock value, Mode mode, Phase oddChannels, Phase evenChannels)
{
  _name = name;
  _clock = value;       
  if (mode == MonoRight)
    _address = 0x49; // set default I2C address for mono right config
  _setMode(mode);
  _setPhase(oddChannels, evenChannels);
}

ES9018::ES9018(String name, Clock value, Mode mode, Phase oddChannels, Phase evenChannels, byte address)
{
  _name = name;
  _clock = value;       
  _address = address;
  _setMode(mode);
  _setPhase(oddChannels, evenChannels);
}

String ES9018::getName()
{
  return _name;
}

boolean ES9018::initialise()
{
  _printDAC();
  Serial.println(F("initialising"));
  _initialised = true;
  bool l;
  bool result = _locked(l);
  _initialised = false;
  if (result) // able to read lock status
  {
    _setInitialised(true);
    // write values that can only be set on init
    if (_writeMode())
      if (_writePhase())
         return true;
  }
  _printDAC();
  Serial.println(F("Initialisation failed"));
  return false;
}

void ES9018::reset()
{
  _setInitialised(false);
}

bool ES9018::getInitialised()
{
  return _initialised;
}

void ES9018::_printDAC()
{
  Serial.print(F("->"));
  Serial.print(_name);
  Serial.print(F(" [ES9018 @"));
  Serial.print(String(_address, HEX));
  Serial.print(F("]: "));
}

void ES9018::_setInitialised(boolean val)
{
  _initialised = val;
  _printDAC();
  if (_initialised)
    Serial.println(F("initialised"));
  else
    Serial.println(F("uninitialised"));
}

ES9018::Mode ES9018::getMode()
{
  return _mode;
}

byte ES9018::getAddress()
{
  return _address;
}

bool ES9018::locked()
{
  bool l;
  if (_locked(l))
    return l;
  else
    return false;
}

bool ES9018::locked(bool &readError)
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

bool ES9018::_locked(bool &lockStatus)
{
  byte status;
  if (_readRegister(27, status))
  {
    lockStatus = status & B00000001;
    return true;
  }
  lockStatus = false;
  return false;
}

boolean ES9018::validSPDIF(bool &status)
{
  byte val;
  if(_readRegister(27, val))
  {
    status = val & B00000100;
    return true;
  }
  return false;
}

bool ES9018::mute()
{
  _printDAC();
  Serial.println(F("muting"));
  bool result = _writeRegisterBits(10,"*******1");              // Set bit zero for reg 10: Mute DACs
  return result;
}

bool ES9018::unmute()
{
  _printDAC();
  Serial.println(F("unmuting"));
  bool result = _writeRegisterBits(10,"*******0");            // Clear bit zero for reg 10: UnMute DACs
  return result;
}

/*
READING THE SAMPLE RATE
 
 The sample rate can be calculated by reading the DPLL 32-bit register. For SPDIF DPLL value 
 is divided by (2^32/Crystal-Frequency). In Buffalo II, the Crystal frequency is 80,000,000 Hz. In 
 Arduino (and other small microprocessors) it is NOT advisable to do floating point math because 
 "it is very slow"; therefore integer math will be used to calculate the sample rate.
 
 The value of 2^32/80,000,000 is 53.687091 (which is a floating point number). If we use the 
 integer part (53 or 54) we get the following results for a 44.1K sample rate signal:  divided by 53 
 the result is 44.677K; divided by 54, the result is 43.849K. Clearly there are large errors from 
 being confined to integer math. The actual result, if we use floating point math and use all the 
 significant digits is 44,105 Hz (the 5 Hz deviation from ideal 44100 Hz is within the specification
 of SPDIF and the tolerances of the crystals and clocks involved)
 
 In order to increase the accuracy of the integer calculation, we can use more of the significant 
 digits of the divisor. I did some evaluation of the DPLL register values for sample rates ranging 
 from 44.1K to 192K and noticed that I could multiply the value of the DPLL number by up to 
 400 without overflowing the 32-bits. Therefore, since we have 32 bit number to work with, we 
 can multiply the DPLL number by 400 and then divide by 400X53.687091=21475. If we do this, 
 we obtain 44.105K which is the same as the exact value.
 
 For I2S input the dpll number is divided by (2^32*64/Crystal-Frequency) Note the 64 factor.
 The value of this is 3435.97 which rounds off nicely to 3436 (which is only 0.0008% error). The
 resultant value for the sample rate is the same whether in spdif or I2S mode.
 */

unsigned long ES9018::sampleRate() 
{
  unsigned long DPLLNum=0;
  // Reading the 4 registers of DPLL one byte at a time and stuffing into a single 32-bit number
  byte val;
  _readRegister(31, val);
  DPLLNum|=val;
  DPLLNum<<=8;
  _readRegister(30, val);
  DPLLNum|=val;
  DPLLNum<<=8;
  _readRegister(29, val);
  DPLLNum|=val;
  DPLLNum<<=8;
  _readRegister(28, val);
  DPLLNum|=val;
  bool spdif;
  validSPDIF(spdif);
  if (spdif) // SPDIF signal
  {
    DPLLNum*=400;    // Calculate SR for SPDIF (part 1)
    DPLLNum/=21475;  // Calculate SR for SDPIF (part 2)
  }
  else 
  {
    if (_clock == Clock80Mhz)
      DPLLNum/=3436;   // Calculate SR for I2S 80MHz clock
    else
      DPLLNum/=2749;   // Calculate SR for I2S 100MHz clock
  }
  return DPLLNum;
}

bool ES9018::_readRegister(byte regAddr, byte &regVal) 
{
  if (!_initialised)
  {
    _printDAC();
    Serial.print(F("Uninitialised Error reading status register "));
    Serial.println(String(regAddr));
    return false;
  }
  if (noI2C)
    return true;
  Wire.beginTransmission(_address); 
  Wire.write(regAddr);
  byte result;           
  result = Wire.endTransmission();
  if (result == 0) // success
  {
    Wire.requestFrom(_address, 1); // request one byte from address
    unsigned long retryUntil = millis() + _readRetryInterval;
    while (!Wire.available())
    {
      if (millis() > retryUntil)
      {
        _printDAC();
        Serial.print(F("timeout reading status register "));
        Serial.println(String(regAddr));
        return false;
      }
    }
    regVal = Wire.read();         // Return the value returned by specified register
/*    
    _printDAC();
    Serial.print(F("read value "));
    Serial.print(String(regVal, BIN));
    Serial.print(F(" from register "));
    Serial.println(String(regAddr));
*/
    return true;
  }
  else if (result == 1)
  {
    _printDAC();
    Serial.print(F("Error reading status register "));
    Serial.print(String(regAddr));
    Serial.println(F(" - data too long to fit in transmit buffer"));
  }
  else if (result == 2)
  {
    _printDAC();
    Serial.print(F("Error reading status register "));
    Serial.print(String(regAddr));
    Serial.println(F(" - received NACK on transmit of address"));
  }
  else if (result == 3)
  {
    _printDAC();
    Serial.print(F("Error reading status register "));
    Serial.print(String(regAddr));
    Serial.println(F(" - received NACK on transmit of data"));
  }
  else if (result == 3)
  {
    _printDAC();
    Serial.print(F("Error reading status register "));
    Serial.print(String(regAddr));
    Serial.println(F(" - received unspecified error"));
  }
  return false;
}

bool ES9018::_writeRegister(byte regAddr, byte regVal)
{
  if (!_initialised)
  {
    _printDAC();
    Serial.print(F("Uninitialised Error writing status register "));
    Serial.println(String(regAddr));
    return false;
  }
  if (noI2C)
    return true;
  _printDAC();
  Serial.print(F("Writing "));
  Serial.print(String(regVal, BIN));
  Serial.print(F(" to register "));
  Serial.println(String(regAddr));
  byte readVal;
  bool readOk = _readRegister(regAddr, readVal);
  if (!readOk)
  {
    Serial.println(F("-Read Error reading current register value- "));
    return false;
  }
  else
  {  
    if (readVal == regVal)
      Serial.println(F("-Write value same as register value- "));
    else
    {
      Wire.beginTransmission(_address); 
      Wire.write(regAddr);               // Specifying the address of register
      int result = Wire.write(regVal);   // Writing the value into the register
      Wire.endTransmission();
      readOk = _readRegister(regAddr, readVal); // confirm write
      if (!readOk)
      {
        Serial.print(F("-Write Error- "));
        Serial.print(F(" could not read written value from register "));
        Serial.println(String(regAddr));
        return false;
      }
      else
      {
        if (readVal != regVal)
        {
          Serial.print(F("-Write Error- "));
          Serial.print(String(readVal, BIN));
          Serial.print(F(" read from register "));
          Serial.println(String(regAddr));
          return false;
        }
        else
          Serial.println(F("-Write Success!- "));
      }
    }
  }
  return true;
}

boolean ES9018::_changeByte(byte &val, String bits)
{
  boolean result = false;
  int l = bits.length();
  if (l != 8)
  {
    Serial.print(F("changebits: "));
    Serial.print(bits);
    Serial.println(F(" must contain 8 characters"));
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
        Serial.print(F("changebits: "));
        Serial.print(bits);
        Serial.print(F(" contains invalid character "));
        Serial.println(c);
        return false;
    }
  }
  return result;
}

bool ES9018::_writeRegisterBits(byte regAddr, String bits) 
{
  byte regVal;
  bool ok = _readRegister(regAddr, regVal);
  if (!ok)
    return false;
  ok = _changeByte(regVal, bits);
  if (!ok)
    // nothing to change
    return true;
  return _writeRegister(regAddr, regVal);
}

bool ES9018::setAttenuation(byte attenuation)
{
  return _writeRegister(0, attenuation) & _writeRegister(1, attenuation) & _writeRegister(2, attenuation) & _writeRegister(3, attenuation) & _writeRegister(4, attenuation) & _writeRegister(5, attenuation) & _writeRegister(6, attenuation) & _writeRegister(7, attenuation);
}

bool ES9018::setAutoMuteLevel(byte level)
{
  _printDAC();
  Serial.println(F("setting auto mute level"));
  byte reg8;  
  if (_readRegister(8, reg8))
  {
    reg8 = reg8 & B10000000; // clear bytes 0-6 which hold the automute level
    level = level & B01111111;     // only first 7 bits of level will be used
    reg8 += level;
    return _writeRegister(8, reg8);
  }
  return false;
}

bool ES9018::setBypassOSF(boolean value)
{
  _printDAC();
  Serial.println(F("setting bypass OSF"));
  if (value)
  {
    return _writeRegisterBits(17, F("*0******"));    // Reg 17: clear bypass oversampling bit in register
  }
  else
  {
    if (_writeRegisterBits(17, F("*11*****")))   // Reg 17: set bypass oversampling bit and Jitter lock bit, normal operation
    {
      delay(50);
      return _writeRegisterBits(17, F("**0*****"));  // Reg 17: clear relock jitter for normal operation
    }
  }
  return false;
}

bool ES9018::setIIRBandwidth(IIR_Bandwidth value)
{
  _printDAC();
  Serial.println(F("setting IIR bandwidth"));
  if (value == IIR_50k)
    return _writeRegisterBits(14, F("*****01*")); // Reg 14: bytes 1-2 hold the IIR Bandwidth (IIR_Normal)
  else
  {
    if (value == IIR_60k)
      return _writeRegisterBits(14, F("*****10*"));
    else 
      if (value == IIR_70k)
        return _writeRegisterBits(14, F("*****11*"));
  }
  return _writeRegisterBits(14, F("*****00*"));  // Normal bandwidth (for PCM)   
}

bool ES9018::setSPDIFMode(SPDIFMode mode)
{
  _printDAC();
  Serial.println(F("setting SPDIF mode"));
  if (mode == SPDIF_Auto)
    return _writeRegisterBits(17, F("****1***"));        // Reg 17: set Auto SPDIF bit 3 in register
  else
    return _writeRegisterBits(17, F("****0***"));        // Reg 17: clear Auto SPDIF bit 3 in register
}

bool ES9018::setSPDIFAutoDeEmphasis(boolean value)
{
  _printDAC();
  Serial.println(F("setting SPDIF auto de-emphasis"));
  if (value)
    return _writeRegisterBits(17, F("***1****"));         // Reg 17: set deemph bit 4 in register
  else
    return _writeRegisterBits(17, F("***0****"));       // Reg 17: clear deemph bit 4 in register
}

bool ES9018::setFIRPhase(Phase phase)
{
  _printDAC();
  Serial.println(F("setting FIR phase"));
  if (phase == AntiPhase)
    return _writeRegisterBits(17, F("******1*"));         // Reg 17: set FIR phase invert bit 1 in register
  else
    return _writeRegisterBits(17, F("******0*"));        // Reg 17: FIR phase invert bit 1 in register
}

bool ES9018::setDeEmphasis(DeEmphasis mode)
{
  _printDAC();
  Serial.println(F("setting de-emphasis"));
  if (mode == DeEmph441k)
    return _writeRegisterBits(11, F("******01"));
  else 
    if (mode == DeEmph48k)
      return _writeRegisterBits(11, F("******10"));
    else
      return _writeRegisterBits(11, F("******00")); // Reg 11: clear bytes 0-1 which hold the automute level (DeEmph 32k)
}

bool ES9018::setJitterReduction(boolean value)
{
  _printDAC();
  Serial.println(F("setting jitter reduction"));
  if (value)
    return _writeRegisterBits(10, F("*****1**"));         // Reg 10: set Jitter Reduction bit 2 in register
  else
    return _writeRegisterBits(10, F("*****0**"));         // Reg 10: clear Jitter Reduction bit 2 in register
}

bool ES9018::setJitterReductionBypass(boolean value)
{
  _printDAC();
  Serial.println(F("setting jitter reduction bypass"));
  if (value)
    _writeRegisterBits(10,F("*****1**"));         // Reg 10: set Jitter Reduction Bypass bit in register
  else
    _writeRegisterBits(10,F("*****0**"));        // Reg 10: clear Jitter Reduction Bypass bit in register
}

bool ES9018::setDPLLMode(DPLLMode mode)
{
  _printDAC();
  Serial.println(F("setting DPLL mode"));
  if (mode == AllowAll)
    return _writeRegisterBits(25, F("******0*"));      // Reg 25: clear DPLLMode bit 1 in register
  else
    return _writeRegisterBits(25, F("******1*"));      // Reg 25: set DPLLMode bit 1 in register
}

bool ES9018::setFIRRollOff(FIR_RollOffMode mode)
{
  _printDAC();
  Serial.println(F("setting FIR rolloff"));
  if (mode == Slow)
    return _writeRegisterBits(14, F("*******0"));      // Reg 14: clear FIR Rollof bit 0 in register
  else
    return _writeRegisterBits(14, F("*******1"));      // Reg 14: set FIR Rollof bit 0 in register
}

bool ES9018::setDPLL(DPLLBandwidth bandwidth)
{
  _printDAC();
  Serial.println(F("setting DPLL"));
  switch(bandwidth)
  {
  case 0:
    // Reg 11: Set DPLL None
    return _writeRegisterBits(11, F("***000**"));
    break;
  case 1:
    // Reg 11: Set DPLL Lowest
    return _writeRegisterBits(11, F("***001**"));
    break;
  case 2:
    // Reg 11: Set DPLL Low
    return _writeRegisterBits(11, F("***010**"));
    break;
  case 3:
    // Reg 11: Set DPLL MediumLow
    return _writeRegisterBits(11, F("***011**"));
    break;
  case 4:
    // Reg 11: Set DPLL Medium
    return _writeRegisterBits(11, F("***100**"));
    break;
  case 5:
    // Reg 11: Set DPLL MediumHigh
    return _writeRegisterBits(11, F("***101**"));
    break;
  case 6:
    // Reg 11: Set DPLL High
    return _writeRegisterBits(11, F("***110**"));
    break;
  case 7:
    // Reg 11: Set DPLL Highest
    return _writeRegisterBits(11, F("***111**"));
    break;
  }
}

bool ES9018::setPhaseB(Phase value)
{
  _printDAC();
  Serial.println(F("setting phase B"));
  if (value == InPhase) 
    return _writeRegister(19, 0xff);
  else
    return _writeRegister(19, 0x00);
}

bool ES9018::setQuantizer(Quantizer value)
{
  _printDAC();
  Serial.println(F("setting quantizer"));
  switch(value)
  {
    case 0:                      
      return _writeRegister(15, 0x00);           // 6-bit quantizer
      break;

    case 1:                      
      return _writeRegister(15, 0x55);           // 7-bit quantizer
      break;

    case 2:                        
      return _writeRegister(15, 0xAA);           // 8-bit quantizer
      break;  

    case 3:                        
      return _writeRegister(15, 0xFF);           // 9-bit quantizer
      break;  
  }
}

bool ES9018::setNotchDelay(NotchDelay value)
{
  _printDAC();
  Serial.println(F("setting notch delay"));
  switch(value)
  {
  case 0:
    return _writeRegister(12, 0x20);    // No notch delay
  case 1:
    return _writeRegister(12, 0x21);    // notch delay=mclk/4
  case 2:
    return _writeRegister(12, 0x23);    // notch delay=mclk/8
  case 3:
    return _writeRegister(12, 0x27);    // notch delay=mclk/16
  case 4:
    return _writeRegister(12, 0x2F);    // notch delay=mclk/32
  case 5:
    return _writeRegister(12, 0x3F);    // notch delay=mclk/64
  }
}

bool ES9018::setDPLL128Mode(DPLL128Mode mode)
{
  _printDAC();
  Serial.println(F("setting DPLL 128 mode"));
  if (mode == UseDPLLSetting)
    return _writeRegisterBits(25, "*******0");  // Reg 25 DPLL128x: Use DPLL setting (D)
  else
    return _writeRegisterBits(25, "*******1");  // Reg 25 DPLL128x: Multiply DPLL by 128
}

bool ES9018::setInputSelect(InputSelect mode)
{
  _printDAC();
  Serial.println(F("setting input select"));
  if (mode == I2SorDSD)
    return _writeRegisterBits(8, "0*******");  // Reg 8 : Use I2S or DSD (D)
  else
    return _writeRegisterBits(8, "1*******");  // Reg 8 : Use SPDIF
}

void ES9018::_setMode(Mode mode)
{
  _mode = mode;
};

void ES9018::_setPhase(Phase oddChannels, Phase evenChannels)
{
  _oddChannels = oddChannels;
  _evenChannels = evenChannels;
};

boolean ES9018::_writeMode()
{
  _printDAC();
  Serial.println(F("writing mode"));
  bool result = false;
  switch (_mode)
  {
    case EightChannel:
      if (_writeRegisterBits(17, F("*******0")))
      {
        result = _writeRegister(14, 0x09); // each DAC source is its own
        if (result)
        {
          _printDAC();
          Serial.println(F("Eight Channel Mode set"));
        }
      }
      break;
    case Stereo:
      if (_writeRegisterBits(17, F("*******0")))
      {
       result = _writeRegister(14, 0xF9); // map dac 8-6, 4-2, 7-5, 3-1
        if (result)
        {
          _printDAC();
          Serial.println(F("Stereo Mode set"));
        }
      }
      break;
    case MonoLeft:
      {
        if (_writeRegisterBits(17, F("0******1")))
        {
          result = true;
          if (result)
          {
          _printDAC();
          Serial.println(F("Mono Left Mode set"));
          }
        }
      }
      break;
    case MonoRight:
      {
        if (_writeRegisterBits(17, F("1******1")))
        {
          result = true;
          if (result)
          {
            _printDAC();
            Serial.println(F("Mono Right Mode set"));
          }
        }
     
    }
  }
  return result;
};

boolean ES9018::_writePhase()
{
  _printDAC();
  Serial.println(F("writing phase"));
  byte val = 0;
  if (_oddChannels == AntiPhase)
  {
    if (_mode == EightChannel)
      _changeByte(val, F("*1*1*1*1"));
    else
      // only need to set channel 1 & 5 phase as reg14 is set so channels 3 & 7 copy their inputs from these channels
       _changeByte(val, F("***1***1"));
  }
  if (_evenChannels == AntiPhase)
  {
    if (_mode == EightChannel)
      _changeByte(val, F("1*1*1*1*"));
    else
      // only need to set channel 2 & 6 phase as reg14 is set so channels 4 & 8 copy their inputs from these channels
      _changeByte(val, F("**1***1*"));
  }
  return _writeRegister(13, val);
};



