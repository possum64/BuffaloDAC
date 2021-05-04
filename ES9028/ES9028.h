/*
  Class library for controlling the ES9028/38 Sabre32 DAC from an Arduino via I2C by David Faulkner
  version 0.1
  Use entirely at your own risk!

  (see the WIRE library for details on connecting an I2C device to an Arduino board. Be aware that most I2C devices use 3.3 volts!)
*/

#include <global.h>
#include "SerialHelper.h"
#include <Wire.h>

#ifndef ES9028_h
#define ES9028_h
#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
  #include "pins_arduino.h"
  #include "WConstants.h"
#endif


class ES9028
{
  public:
    enum Mode{MonoLeft=0, MonoRight=1, Stereo=2, EightChannel=3, DualLeft=4, DualRight=5};
    enum Phase{InPhase, AntiPhase};
    enum OscillatorDrive{OSC_ShutDown=0, OSC_QuarterBias=1, OSC_HalfBias=2, OSC_ThreeQuarterBias=3, OSC_FullBias=4};
    enum ClockGear{XIN=0, XIN_Div2=1, XIN_Div4=2, XIN_Div8=3};
    enum SPDIFUserBits{SPDIF_PresentUserBits=0, SPDIF_PresentChannelBits=1};
    enum SPDIFDataFlag{SPDIF_IgnoreDataFlag=0, SPDIF_MuteIfDataFlagSet=1};
    enum SPDIFValidFlag{SPDIF_IgnoreValidFlag=0, SPDIF_MuteIfInvalidValidFlag=1};
    enum AutoSelect{AutoSelect_DSD_SPDIF_SERIAL=0, AutoSelect_SPDIF_SERIAL=1, AutoSelect_DSD_SERIAL=2, AutoSelect_Disable=3};
    enum InputSelect{InputSelect_DSD=0, InputSelect_SPDIF=2, InputSelect_SERIAL=3};
    enum AutoMute{AutoMute_MuteAndRampToGnd=0, AutoMute_RampToGnd=1, AutoMute_Mute=2, AutoMute_None=3};
    enum Bits{Bits_Default=0, Bits_32=1, Bits_24=2, Bits_16=3};
    enum SerialMode{RightJustified=0, LeftJustified=1, I2S_Mode=2};
    enum DeEmphSelect{DeEmpESelect_48khz=0, DeEmpESelect_441khz=1, DeEmpESelect_32khz=2};
    enum FilterShape{Filter_Brickwall=0, Filter_Hybrid=1, Filter_Apodizing=2, Filter_SlowMinPhase=3, Filter_FastMinPhase=4, Filter_SlowLinPhase=5, Filter_FastLinPhase=6};
    enum IIR_Bandwidth{IIR_70k=0, IIR_60k=1, IIR_50k=2, IIR_4744k=3};
    enum GPIO{GPIO_Automute=0, GPIO_Lock=1, GPIO_VolumeMin=2, GPIO_CLK=3, GPIO_Interrupt=4, GPIO_ADC_CLK=5, GPIO_ForceLow=6, GPIO_StandardInput=7, GPIO_InputSelect=8, GPIO_MuteAll=9, GPIO_ADC_Input=10, GPIO_SoftStartComplete=11, GPIO_ForceHigh=12};
    enum MasterDiv{MDIV_MCLK2=0, MDIV_MCLK4=1, MDIV_MCLK8=2, MDIV_MCLK16=3};
    enum LockSpeed{LS_16384=0, LS_8192=0, LS_5461=1, LS_4096=2, LS_3276=3, LS_2730=4, LS_2340=5, LS_2048=6, LS_1820=7, LS_1638=8, LS_1489=9, LS_1365=10, LS_1260=11, LS_1170=12, LS_1092=13, LS_1024=14};
    enum SPDIFInput{SPDIF_Data_CLK=0, SPDIF_Data1=1, SPDIF_Data2=2, SPDIF_Data3=3, SPDIFData_4=4, SPDIFData_5=5, SPDIFData_6=6, SPDIF_Data7=7, SPDIF_Data8=8, SPDIF_GPIO1=9, SPDIF_GPIO2=10, SPDIF_GPIO3=11, SPDIF_GPIO4=12};
    enum DpllBandwidth{DPLL_Lowest=0, DPLL_VeryLow=1, DPLL_Lower=2, DPLL_Low=3, DPLL_Default=4, DPLL_LowToMediumLow=5, DPLL_MediumLow=6, DPLL_Medium=7, DPLL_MediumToMEdiumHigh=8, DPLL_MediumHigh=9, DPLL_HighToMediumHigh=10, DPLL_High=11 , DPLL_Higher=12, DPLL_HigherToVeryHigh=13, DPLL_VeryHigh=14, DPLL_Highest=15};
    enum SoftStart{SS_RampToGround=0, SS_RampToAVCC=1};
    enum VolumeMode{Volume_Independent=0, Volume_UseChannel1=1};
    enum FIRCoeffStage{Coeff_Stage1=0, Coeff_Stage2=1};
    enum FIRStage2{Stage2_Sine=0, Stage2_Cosine=1};
    enum Input{Input_1=0, Input_2=1, Input_3=2, Input_4=3, Input_5=4, Input_6=5, Input_7=6, Input_8=7};
    enum Gain{Gain_None=0, Gain_18db=1};
    enum ChipType{Chip_Unknown=0, Chip_ES9028PRO=1, Chip_ES9038PRO=2};
    enum SignalType{Signal_DoP=0, Signal_SPDIF=1, Signal_I2S=2, Signal_DSD=3, Signal_NONE=4};
    
    ES9028(String name);                            // default to 8 channel mode with default I2C address 0x48
    ES9028(String name, Mode mode);
    ES9028(String name, byte addr);                 // default to 8 channel mode with default I2C address 0x48
    ES9028(String name, Mode mode, byte addr);    
    byte clock = 10;				                        // value of clock used (in 10s of MHz). 10 = 100MHz.
    bool noI2C = false;                             // set to true for debugging/development of code when Arduino not connected via I2C to DAC
    bool initialise();                              // writes mode and phase values. Other registers can only be changed after this method is called.
    bool getInitialised();  
    ES9028::Mode getMode();
    bool reset();
    String getName();
    byte getAddress();                              // returns the I2C address
    bool setOscillatorDrive(OscillatorDrive val);   // Configures a clock divider network that can reduce the power consumption of the chip
    bool setClockGear(ClockGear val);               // Software configurable hardware reset with the ability to reset the design to its initial power-on configuration.
    bool setSPDIFUserBits(SPDIFUserBits val);       // Setting user_bits will present the SPDIF user bits on the read-only register interface instead of the default channel status bits.
    bool setSPDIFDataFlag(SPDIFDataFlag val);       // Configures the SPDIF decoder to ignore the �data� flag in the channel status bits.
    bool setSPDIFValidFlag(SPDIFValidFlag val);     // Configures the SPDIF decoder to ignore the �valid� flag in the SPDIF stream.
    bool setAutoSelect(AutoSelect val);             // Allows the SABRE DAC to automatically select between either serial, SPDIF or DSD input formats
    bool setInputSelect(InputSelect val);           // Configures the SABRE DAC to use a particular input decoder if auto_select is disabled.
    bool setAutoMute(AutoMute val);                 // Configures the automute state machine
    bool setSerialBits(Bits val);                   // Selects how many bits consist of a data word in the serial data stream.
    bool setSerialLength(Bits val);                 // Selects how many DATA_CLK pulses exist per data word.
    bool setSerialMode(SerialMode val);             // Configures the type of serial data.
    bool setAutomuteTime(byte val);                 // Configures the amount of time the audio data must remain below the automute_level before an automute condition is flagged. Defaults to 0 which disables automute
    bool setAutomuteLevel(byte val);                // Note: This register works in tandem with automute_time to create the automute condition.
    bool enableAutoDeEmph();                        // Automatically engages the de-emphasis filters when SPDIF data is provides and the SPDIF channel status bits contains valid de-emphasis settings
    bool disableAutoDeEmph();                       // disables AutoDeEmph
    bool enableDeEmph();                            // enables the built-in de-emphasis filters.
    bool disableDeEmph();                           // disables the built-in de-emphasis filters.
    bool setDeEmphSelect(DeEmphSelect val);         // Selects which de-emphasis filter is used.
    bool setVolumeRate(byte val);                   // Selects a volume ramp rate to use when transitioning between different volume levels. The volume ramp rate is measured in decibels per second (dB/s). Volume rate is in the range 0-7.
    bool setFilterShape(FilterShape val);           // Selects the type of filter to use during the 8x FIR interpolation phase.
    bool setIIR_Bandwidth(IIR_Bandwidth val);       // Selects the type of filter to use during the 8x IIR interpolation phase.
    bool mute();                                    // Mutes all 8 channels of the SABRE DAC.
    bool unmute();                                  // Unmutes all 8 channels of the SABRE DAC.
    bool setGPIO1(GPIO val);                        // GPIO 1 Configuration
    bool setGPIO2(GPIO val);                        // GPIO 2 Configuration
    bool setGPIO3(GPIO val);                        // GPIO 3 Configuration
    bool setGPIO4(GPIO val);                        // GPIO 4 Configuration
    bool enableMasterMode();                        // Enables master mode which causes the SABRE DAC to derive the DATA_CLK and DATA1 signals when in I2S mode
    bool disableMasterMode();                       // Disables master mode
    bool setMasterDiv(MasterDiv val);               // Sets the frame clock (DATA1) and DATA_CLK frequencies when in master mode.
    bool enable128fsMode();                         // Enables operation of the DAC while in synchronous mode with a 128*FSR MCLK in PCM normal or OSF bypass mode only.
    bool disable128fsMode();                        // Disables operation of the DAC while in synchronous mode with a 128*FSR MCLK in PCM normal or OSF bypass mode only.
    bool setLockSpeed(LockSpeed val);               // Sets the number of audio samples required before the DPLL and jitter eliminator lock to the incoming signal.
    bool setSPDIFInput(SPDIFInput val);             // Selects which input to use when decoding SPDIF data. Note: If using a GPIO the GPIO configuration must be set to an input.
    bool invertGPIO1();                             // inverts the GPIO 1 output when set.
    bool invertGPIO2();                             // inverts the GPIO 2 output when set.
    bool invertGPIO3();                             // inverts the GPIO 3 output when set.
    bool invertGPIO4();                             // inverts the GPIO 4 output when set.
    bool nonInvertGPIO1();                          // Non-inverts the GPIO 1 output when set.
    bool nonInvertGPIO2();                          // Non-inverts the GPIO 2 output when set.
    bool nonInvertGPIO3();                          // Non-inverts the GPIO 3 output when set.
    bool nonInvertGPIO4();                          // Non-inverts the GPIO 4 output when set.
    bool setDpllBandwidthSerial(DpllBandwidth val); // Sets the bandwidth of the DPLL when operating in I2S/SPDIF mode
    bool setDpllBandwidthDSD(DpllBandwidth val);    // Sets the bandwidth of the DPLL when operating in DSD mode
    bool enableJitterEliminator();                  // Enables the jitter eliminator and DPLL circuitry
    bool disableJitterEliminator();                 // Disables the jitter eliminator and DPLL circuitry
    bool enableTHDcompensation();                   // Enables THD compensation 
    bool disableTHDcompensation();                  // Disables THD compensation
    bool enableDither();                            // Enables dither in the noise shaped modulators
    bool disableDither();                           // Disables dither in the noise shaped modulators
    bool setSoftStart(SoftStart val);               // The Sabre DAC initializes both DAC and DACB to GND, and then ramps up the signal to AVCC/2. DAC and DACB remain in phase until the ramp is complete. Soft_start controls the ramp operation and defaults to 1�b1 (ramp to AVCC /2)
    bool enableSoftStopOnUnlock();                  // Automatically ramps the output low when lock is lost
    bool disableSoftStopOnUnlock();                 // Does not automatically ramp the output low when lock is lost
    bool setSoftStartTime(byte val);                // Sets the amount of time it takes to perform a soft-start ramp. The value is valid from 0 to 20 (inclusive).
    bool setGPIOSelect1(InputSelect val);           // Selects which input type will be selected when GPIOX = 1�b1
    bool setGPIOSelect2(InputSelect val);           // Selects which input type will be selected when GPIOX = 1�b1
    bool setVolumeMode(VolumeMode val);             // Force all eight channels to use the volume coefficients from channel 1.
    bool enableVolumeLatching();                    // enables the volume control registers (default)
    bool disableVolumeLatching();                   // disables latching of the volume control registers
    bool setVolume1(byte val);                      // Channel 1 - Default of 8�d0 -0dB to -127.5dB with 0.5dB steps
    bool setVolume2(byte val);                      // Channel 2 - Default of 8�d0 -0dB to -127.5dB with 0.5dB steps
    bool setVolume3(byte val);                      // Channel 3 - Default of 8�d0 -0dB to -127.5dB with 0.5dB steps
    bool setVolume4(byte val);                      // Channel 4 - Default of 8�d0 -0dB to -127.5dB with 0.5dB steps
    bool setVolume5(byte val);                      // Channel 5 - Default of 8�d0 -0dB to -127.5dB with 0.5dB steps
    bool setVolume6(byte val);                      // Channel 6 - Default of 8�d0 -0dB to -127.5dB with 0.5dB steps
    bool setVolume7(byte val);                      // Channel 7 - Default of 8�d0 -0dB to -127.5dB with 0.5dB steps
    bool setVolume8(byte val);                      // Channel 8 - Default of 8�d0 -0dB to -127.5dB with 0.5dB steps
    bool setMasterTrim(unsigned long val);          // A 32-bit signed value that sets the 0dB level for all volume controls. Defaults to full-scale (32�h7FFFFFFF).
    bool setTHDCompensationC2(int val);             // A 16-bit signed coefficient for correcting for the second harmonic distortion. Defaults to 16�d0.
    bool setTHDCompensationC3(int val);             // A 16-bit signed coefficient for correcting for the third harmonic distortion. Defaults to 16�d0.
    bool setFIRCoeffStage(FIRCoeffStage val);       // Selects which stage of the filter to write.
    bool setFIRCoeffAddr(byte val);                 // Selects the coefficient address when writing custom coefficients for the oversampling filter
    bool setFIRCoeff(unsigned long val);            // A 32-bit signed filter coefficient that will be written to the address defined in prog_coeff_addr.
    bool enableFIRExternalBypassOSF();              // enables the use of an external 8x upsampling filter, bypassing the internal interpolating FIR filter.
    bool disableFIRExternalBypassOSF();             // uses the internal interpolating FIR filter.
    bool enableFIRExtendedFilterLength();           // uses an extended 256-tap first stage filter at the expense of disabling oversampling on channels 3-8. This mode should only be used when in stereo operation and with channel mapping set appropriately
    bool disableFIRExternalFilterLength();          // uses the standard 128-tap first stage filter when in fast rolloff mode (default)
    bool enableFIRProgExt();                        // prog_coeff_addr maps to coefficients 0-127 (default)
    bool disableFIRProgExt();                       // prog_coeff_addr maps to coefficients 128-255
    bool setFIRStage2(FIRStage2 val);               // Selects the symmetry of the stage 2 oversampling filter.
    bool enableFIRProgCoeffWrite();                 // Enables writing to the programmable coefficient RAM.
    bool disableFIRProgCoeffWrite();                // Disables writing to the programmable coefficient RAM.
    bool enableFIRProgCoeff();                      // Uses the coefficients programmed via prog_coeff_data
    bool disableFIRProgCoeff();                     // Uses a built-in filter selected by filter_shape (default)
    bool mapInputs(Input dac1, Input dac2, Input dac3, Input dac4, Input dac5, Input dac6, Input dac7, Input dac8);
    bool setProgrammableNCO(unsigned long val);     // An unsigned 32-bit quantity that provides the ratio between MCLK and DATA_CLK. This value can be used to generate arbitrary DATA_CLK frequencies in master mode.
    bool setChannelGain(Gain dac1, Gain dac2, Gain dac3, Gain dac4, Gain dac5, Gain dac6, Gain dac7, Gain dac8);  // Note: The +18dB gain only works in PCM mode and is applied prior to the channel mapping.
    bool GPIO_Status(bool &gpio1, bool &gpio2, bool &gpio3, bool &gpio4);
    ES9028::ChipType chipType();                    // returns whether the chip is an ES2028/38 or unknown
    bool setMode(Mode val);                         // Stereo, 8 channel, or mono left/right operation 
    bool getAutomuted(bool &automuteStatus);        // returns true if Automute read successful and sets the parameter reference to true if Automute has been flagged and is active
    bool locked();                                  // returns true if DPLL is locked to the incoming audio sample rate, or the Sabre is in master mode, 128fs_mode or NCO mode mode
    bool locked(bool &readError);
    bool dopValid();                                // returns true if the DoP decoder has detected a valid DoP signal on the I2S or SPDIF inputs.
    bool spdifValid();                              // returns true if the SPDIF decoder has decoded a sequence of valid SPDIF frames.
    bool i2sValid();                                // returns true if the I2S decoder has detected a valid frame clock and bit clock arrangement.
    bool dsdValid();                                // returns true if the DSD decoder is being used as a fallback option if I2S and SPDIF have both failed to decode their respective input signals.
    ES9028::SignalType getSignalType();             // returns signal type.
    unsigned long dpllNumber();                     // returns the ratio between the MCLK and the audio clock rate once the DPLL has acquired lock
    unsigned long getSampleRate();                  // returns the sample rate
    bool setAttenuation(byte attenuation);          // sets the same attenuation for each DAC
  private:
    Mode _mode = EightChannel;                      // default is eight channel mode
    String _name;
    byte _address = 0x48;                           // set default I2C address
    bool _initialised = false;
    bool _locked(bool &lockStatus);
    ChipType _chipType = Chip_Unknown;
    const int _readRetries = 5;                     // _readRegister read error retries
    Phase _oddChannels = InPhase;
    Phase _evenChannels = InPhase;

    bool _readRegister(byte regAddr, byte &regVal); 
    bool _writeRegister(byte regAddr, byte regVal); // writes the specified register value to the specified DAC register via I2C
    bool _writeRegisterBits(byte regAddr, String bits); 
    bool _writeMode();
    bool _writePhase();
    bool _invalidSetting();
    bool _changeByte(byte &val, String bits);
    void _setInitialised(boolean val);
    void _printDAC();
    void _printDAC(Msg::Level level);
    bool _setInputs(byte reg, Input inputA, Input inputB);
    bool _getChipType(ChipType &chip);
};

#endif

