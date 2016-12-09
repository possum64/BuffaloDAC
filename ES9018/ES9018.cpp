#include "ES9018.h"

ES9018::ES9018()
{
}

ES9018::ES9018(DACMode mode)
{
  _setMode(mode);
}

ES9018::ES9018(DACMode mode, DACPhase oddChannels, DACPhase evenChannels)
{
  if (mode == MonoRight)
    _address = 0x49; // set default I2C address for mono right config
  _setMode(mode);
  _setPhase(oddChannels, evenChannels);
}

ES9018::ES9018(DACMode mode, DACPhase oddChannels, DACPhase evenChannels, byte address)
{
  _address = address;
  _setMode(mode);
  _setPhase(oddChannels, evenChannels);
}

void ES9018::init()
{
  _regValChanged0 = true;
  _regValChanged1 = true;   
  _regValChanged2 = true;      
  _regValChanged3 = true;     
  _regValChanged4 = true;      
  _regValChanged5 = true;
  _regValChanged6 = true;
  _regValChanged7 = true;
  _regValChanged8 = true;
  _regValChanged10 = true;
  _regValChanged11 = true;
  _regValChanged12 = true;
  _regValChanged13 = true;
  _regValChanged14 = true;
  _regValChanged15 = true;
  _regValChanged17 = true;
  _regValChanged19 = true;
  _regValChanged25 = true;
  _initialised = true;
  _writeRegisters();
}

DACMode ES9018::getMode()
{
  return _mode;
}

byte ES9018::getAddress()
{
  return _address;
}

boolean ES9018::locked()
{
  byte status = _readStatusRegister(27);
  return status & B00000001;
}

boolean ES9018::validSPDIF()
{
  byte status = _readStatusRegister(27);
  return status & B00000100;
}

void ES9018::mute()
{
  byte reg10 = _reg10;
  bitSet(reg10,0);              // Set bit zero for reg 10: Mute DACs
  _setReg10(reg10);
}

void ES9018::unmute()
{
  byte reg10 = _reg10;
  bitClear(reg10,0);            // Set bit zero for reg 10: UnMute DACs
  _setReg10(reg10);
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
  DPLLNum|=_readStatusRegister(31);
  DPLLNum<<=8;
  DPLLNum|=_readStatusRegister(30);
  DPLLNum<<=8;
  DPLLNum|=_readStatusRegister(29);
  DPLLNum<<=8;
  DPLLNum|=_readStatusRegister(28);
  if (validSPDIF()) // SPDIF signal
  {
    DPLLNum*=400;    // Calculate SR for SPDIF (part 1)
    DPLLNum/=21475;  // Calculate SR for SDPIF (part 2)
  }
  else 
  {
    DPLLNum/=3436;   // Calculate SR for I2S 80MHz clock
  }
  return DPLLNum;
}



byte ES9018::_readStatusRegister(byte regAddr) 
{
  Wire.beginTransmission(_address); 
  Wire.write(regAddr);           
  Wire.endTransmission();
  Wire.requestFrom(_address, 1); // request one byte from address
  if(Wire.available())
    return Wire.read();         // Return the value returned by specified register
  else
    return 0;
}

void ES9018::setAttenuation(byte attenuation)
{
  _setReg0(attenuation);
  _setReg1(attenuation);
  _setReg2(attenuation);
  _setReg3(attenuation);
  _setReg4(attenuation);
  _setReg5(attenuation);
  _setReg6(attenuation);
  _setReg7(attenuation);
}

void ES9018::setAutoMuteLevel(byte level)
{
  byte reg8 = _reg8 & B10000000; // clear bytes 0-6 which hold the automute level
  level = level & B01111111;     // only first 7 bits of level will be used
  reg8 += level;
  _setReg8(reg8);
}

void ES9018::setBypassOSF(boolean value)
{
  byte reg17 = _reg17;
  if (value)
  {
    bitClear(reg17, 6);       // Reg 17: clear bypass oversampling bit in register
    _setReg17(reg17);         // Reg 17: bypass OSF off
  }
  else
  {
    bitSet(reg17,6);          // Reg 17: set bypass oversampling bit in register
    bitSet(reg17,5);          // Reg 17: set Jitter lock bit, normal operation
    _setReg17(reg17);         // Reg 17: bypass OSF on, force relock
    delay(50);
    bitClear(reg17, 5);       // Reg 17: clear relock jitter for normal operation
    _setReg17(reg17);         // Reg 17: Jitter eliminator Normal operation 
  }
}

void ES9018::setIIRBandwidth(IIR_Bandwidth value)
{
  byte reg14 = _reg14 & B1111001; // Reg 14: mask bytes 1-2 which hold the IIR Bandwidth (IIR_Normal)
  if (value == IIR_50k)
    reg14 += B00000010;
  else
  {
    if (value == IIR_60k)
      reg14 += B00000100;
    else
      reg14 += B00000110;
  }
  _setReg14(reg14);        
}

void ES9018::setSPDIFMode(SPDIFMode mode)
{
  byte reg17 = _reg17;
  if (mode == SPDIF_Auto)
    bitSet(reg17, 3);         // Reg 17: set Auto SPDIF bit in register
  else
    bitClear(reg17, 3);       // Reg 17: clear Auto SPDIF bit in register
  _setReg17(reg17);        
}

void ES9018::setSPDIFAutoDeEmphasis(boolean value)
{
  byte reg17 = _reg17;
  if (value)
    bitSet(reg17, 4);         // Reg 17: set deemph bit in register
  else
    bitClear(reg17, 4);       // Reg 17: clear deemph bit in register
  _setReg17(reg17);        
}

void ES9018::setFIRPhase(DACPhase phase)
{
  byte reg17 = _reg17;
  if (phase == AntiPhase)
    bitSet(reg17, 1);         // Reg 17: set FIR phase invert bit in register
  else
    bitClear(reg17, 1);       // Reg 17: FIR phase invert bit in register
  _setReg17(reg17);        
}

void ES9018::setDeEmphasis(DACDeEmphasis mode)
{
  byte reg11 = _reg11 & B1111100; // Reg 11: clear bytes 0-1 which hold the automute level (DeEmph 32k)
  if (mode == DeEmph441k)
    reg11 += B00000001;
  else
  {
    if (mode == DeEmph48k)
      reg11 += B00000010;
  }
  _setReg11(reg11);        
}

void ES9018::setJitterReduction(boolean value)
{
  byte reg10 = _reg10;
  if (value)
    bitSet(reg10, 2);         // Reg 10: set Jitter Reduction bit in register
  else
    bitClear(reg10, 2);       // Reg 10: clear Jitter Reduction bit in register
  _setReg10(reg10);        
}

void ES9018::setJitterReductionBypass(boolean value)
{
  byte reg10 = _reg10;
  if (value)
    bitSet(reg10, 3);         // Reg 10: set Jitter Reduction Bypass bit in register
  else
    bitClear(reg10, 3);       // Reg 10: clear Jitter Reduction Bypass bit in register
  _setReg10(reg10);        
}

void ES9018::setDPLLMode(DPLLMode mode)
{
  byte val = _reg25;
  if (mode == AllowAll)
    bitClear(val, 1);
  else
    bitSet(val, 1);
  _setReg25(val);
}

void ES9018::setFIRRollOff(FIR_RollOffMode mode)
{
  byte val = _reg14;
  if (mode == Slow)
    bitClear(val, 1);
  else
    bitSet(val, 1);
  _setReg14(val);
}

void ES9018::setDPLL(DPLLBandwidth bandwidth)
{
  switch(bandwidth)
  {
  case 0:
    _setReg11(0x85);   // Reg 11: Set DPLL None
    break;
  case 1:
    _setReg11(0x89);   // Reg 11: Set DPLL Lowest
    break;
  case 2:
    _setReg11(0x8D);   // Reg 11: Set DPLL Low
    break;
  case 3:
    _setReg11(0x91);   // Reg 11: Set DPLL MediumLow
    break;
  case 4:
    _setReg11(0x95);   // Reg 11: Set DPLL Medium
    break;
  case 5:
    _setReg11(0x99);   // Reg 11: Set DPLL MediumHigh
  case 6:
    _setReg11(0x9D);   // Reg 11: Set DPLL High
  }
}

void ES9018::setPhaseB(DACPhase value)
{
  if (value == InPhase) 
    _setReg19(0xff);
  else
    _setReg19(0x00);
}

void ES9018::setQuantizer(DACQuantizer value)
{
  switch(value)
  {
    case 0:                      
      _setReg15(0x00);           // 6-bit quantizer
      break;

    case 1:                      
      _setReg15(0x55);           // 7-bit quantizer
      break;

    case 2:                        
      _setReg15(0xAA);           // 8-bit quantizer
      break;  

    case 3:                        
      _setReg15(0xFF);           // 9-bit quantizer
      break;  
  }
}

void ES9018::setNotchDelay(DACNotchDelay value)
{
  switch(value)
  {
  case 0:
    _setReg12(0x20);    // No notch delay
    break;
/*  case 1:
    _setReg12(0x21);    // notch delay=mclk/4
    break;
  case 2:
    _setReg12(0x23);    // notch delay=mclk/8
    break;
  case 3:
    _setReg12(0x27);    // notch delay=mclk/16
    break;  
  case 4:
    _setReg12(0x2F);    // notch delay=mclk/32
    break;
  case 5:
    _setReg12(0x3F);    // notch delay=mclk/64
*/    break; 
  }
}

void ES9018::setDPLL128Mode(DPLL128Mode mode)
{
  byte val = _reg25;
  if (mode == UseDPLLSetting)
    bitClear(val, 0);
  else
    bitSet(val, 0);
  _setReg25(val);
}

void ES9018::setInputMode(DACInputMode mode)
{
  byte val = _reg8;
  if (mode == I2SorDSD)
    bitClear(val, 0);
  else
    bitSet(val, 0);
  _setReg8(val);
}

void ES9018::_writeRegisters()
{
  _writeReg0();
  _writeReg1();
  _writeReg2();
  _writeReg3();
  _writeReg4();
  _writeReg5();
  _writeReg6();
  _writeReg7();
  _writeReg8();
  _writeReg10();
  _writeReg11();
  _writeReg12();
  _writeReg13();
  _writeReg14();
  _writeReg15();
  _writeReg17();
  _writeReg19();
  _writeReg25();
}

void ES9018::_setMode(DACMode mode)
{
  _mode = mode;
  if (mode == EightChannel)
    _setReg14(0x09); // each DAC source is its own
  else
    _setReg14(0xF9); // Source of DAC3 is DAC1, DAC4 is DAC2, DAC7 is DAC5, DAC8 is DAC6
};

void ES9018::_setPhase(DACPhase oddChannels, DACPhase evenChannels)
{
  byte val = 0x00;
  if (oddChannels == AntiPhase)
  {
    if (getMode() == EightChannel)
      val += 0x55;
    else
      // only need to set channel 1 & 5 phase as reg14 is set so channels 3 & 7 copy their inputs from these channels
      val += 0x11;
  }
  if (evenChannels == AntiPhase)
  {
    if (getMode() == EightChannel)
      val += 0xAA;
    else
      // only need to set channel 2 & 6 phase as reg14 is set so channels 4 & 8 copy their inputs from these channels
      val += 0x22;
  }
  _setReg13(val);
};

void ES9018::_setReg0(byte val)
{
  if (_reg0 != val)
  {
    _reg0 = val;
    _regValChanged0 = true;
    _writeReg0();
  }
};

void ES9018::_setReg1(byte val)
{
  if (_reg1 != val)
  {
    _reg1 = val;
    _regValChanged1 = true;
    _writeReg1();
  }
};

void ES9018::_setReg2(byte val)
{
  if (_reg2 != val)
  {
    _reg2 = val;
    _regValChanged2 = true;
    _writeReg2();
  }
};

void ES9018::_setReg3(byte val)
{
  if (_reg3 != val)
  {
    _reg3 = val;
    _regValChanged3 = true;
    _writeReg3();
  }
};

void ES9018::_setReg4(byte val)
{
  if (_reg4 != val)
  {
    _reg4 = val;
    _regValChanged4 = true;
    _writeReg4();
  }
};

void ES9018::_setReg5(byte val)
{
  if (_reg5 != val)
  {
    _reg5 = val;
    _regValChanged5 = true;
    _writeReg5();
  }
};

void ES9018::_setReg6(byte val)
{
  if (_reg6 != val)
  {
    _reg6 = val;
    _regValChanged6 = true;
    _writeReg6();
  }
};

void ES9018::_setReg7(byte val)
{
  if (_reg7 != val)
  {
    _reg7 = val;
    _regValChanged7 = true;
    _writeReg7();
  }
};

void ES9018::_setReg8(byte val)
{
  if (_reg8 != val)
  {
    _reg8 = val;
    _regValChanged8 = true;
    _writeReg8();
  }
};

void ES9018::_setReg10(byte val)
{
  if (_reg10 != val)
  {
    _reg10 = val;
    _regValChanged10 = true;
    _writeReg10();
  }
};

void ES9018::_setReg11(byte val)
{
  if (_reg11 != val)
  {
    _reg11 = val;
    _regValChanged11 = true;
    _writeReg11();
  }
};

void ES9018::_setReg12(byte val)
{
  if (_reg12 != val)
  {
    _reg12 = val;
    _regValChanged12 = true;
    _writeReg12();
  }
};

void ES9018::_setReg13(byte val)
{
  if (_reg13 != val)
  {
    _reg13 = val;
    _regValChanged13 = true;
    _writeReg13();
  }
};

void ES9018::_setReg14(byte val)
{
  if (_reg14 != val)
  {
    _reg14 = val;
    _regValChanged14 = true;
    _writeReg14();
  }
};

void ES9018::_setReg15(byte val)
{
  if (_reg15 != val)
  {
    _reg15 = val;
    _regValChanged15 = true;
    _writeReg15();
  }
};

void ES9018::_setReg17(byte val)
{
  if (_reg17 != val)
  {
    _reg17 = val;
    _regValChanged17 = true;
    _writeReg17();
  }
};

void ES9018::_setReg19(byte val)
{
  if (_reg19 != val)
  {
    _reg19 = val;
    _regValChanged19 = true;
    _writeReg19();
  }
};

void ES9018::_setReg25(byte val)
{
  if (_reg25 != val)
  {
    _reg25 = val;
    _regValChanged25 = true;
    _writeReg25();
  }
};

int ES9018::_writeRegister(byte regAddr, byte regVal)
{
  Wire.beginTransmission(_address); 
  Wire.write(regAddr);               // Specifying the address of register
  int result = Wire.write(regVal);   // Writing the value into the register
  Wire.endTransmission();
  Serial.println("DAC" + String(_address, HEX) + ": " + String(result) + " bytes of " +String(regVal, BIN) + " written to register" + String(regAddr));
  return result;
}

void ES9018::_writeReg0()
{
  if (_initialised && _regValChanged0)
  {
    _writeRegister(0x00, _reg0);
    _regValChanged0 = false;
  }
}

void ES9018::_writeReg1()
{
  if (_initialised && _regValChanged1)
  {
    _writeRegister(0x01, _reg1);
    _regValChanged1 = false;
  }
}

void ES9018::_writeReg2()
{
  if (_initialised && _regValChanged2)
  {
    _writeRegister(0x02, _reg2);
    _regValChanged2 = false;
  }
}

void ES9018::_writeReg3()
{
  if (_initialised && _regValChanged3)
  {
    _writeRegister(0x03, _reg3);
    _regValChanged3 = false;
  }
}

void ES9018::_writeReg4()
{
  if (_initialised && _regValChanged4)
  {
    _writeRegister(0x04, _reg4);
    _regValChanged4 = false;
  }
}

void ES9018::_writeReg5()
{
  if (_initialised && _regValChanged5)
  {
    _writeRegister(0x05, _reg5);
    _regValChanged5 = false;
  }
}

void ES9018::_writeReg6()
{
  if (_initialised && _regValChanged6)
  {
    _writeRegister(0x06, _reg6);
    _regValChanged6 = false;
  }
}

void ES9018::_writeReg7()
{
  if (_initialised && _regValChanged7)
  {
    _writeRegister(0x07, _reg7);
    _regValChanged7 = false;
  }
}

void ES9018::_writeReg8()
{
  if (_initialised && _regValChanged8)
  {
    _writeRegister(0x08, _reg8);
    _regValChanged8 = false;
  }
}

void ES9018::_writeReg10()
{
  if (_initialised && _regValChanged10)
  {
    _writeRegister(0x0A, _reg10);
    _regValChanged10 = false;
  }
}

void ES9018::_writeReg11()
{
  if (_initialised && _regValChanged11)
  {
    _writeRegister(0x0B, _reg11);
    _regValChanged11 = false;
  }
}

void ES9018::_writeReg12()
{
  if (_initialised && _regValChanged12)
  {
    _writeRegister(0x0C, _reg12);
    _regValChanged12 = false;
  }
}

void ES9018::_writeReg13()
{
  if (_initialised && _regValChanged13)
  {
    _writeRegister(0x0D, _reg13);
    _regValChanged13 = false;
  }
}

void ES9018::_writeReg14()
{
  if (_initialised && _regValChanged14)
  {
    _writeRegister(0x0E, _reg14);
    _regValChanged14 = false;
  }
}

void ES9018::_writeReg15()
{
  if (_initialised && _regValChanged15)
  {
    _writeRegister(0x0F, _reg15);
    _regValChanged15 = false;
  }
}

void ES9018::_writeReg17()
{
  if (_initialised && _regValChanged17)
  {
    _writeRegister(0x11, _reg17);
    _regValChanged17 = false;
  }
}

void ES9018::_writeReg19()
{
  if (_initialised && _regValChanged19)
  {
    _writeRegister(0x13, _reg19);
    _regValChanged19 = false;
  }
}

void ES9018::_writeReg25()
{
  if (_initialised && _regValChanged25)
  {
    _writeRegister(0x19, _reg25);
    _regValChanged25 = false;
  }
}

