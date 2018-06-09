# BuffaloDAC
Class libraries for controlling the ES9018/28/38 Sabre32 DACs from an Arduino via I2C (the ES9028 library also works with the ES9038 chip as the control logic is the same)

Gives access to majority of the chips capabilities and provides comprehensive debug/diagnostic capabilities

(see the WIRE library for details on connecting an I2C device to an Arduino board. Be aware that most I2C devices, including the Sabre DACs use 3.3 volts! - whereas Arduinos use 5 volts. The Sabre DAC I2S input is supposedly 5 volt-tolerant, but you should use an I2C isolator in any case to prevent noise from the Arduino interfering with the DAC. TIP: Keep yur I2C leads relatively short to avoid unreliable communications)
