# BuffaloDAC
Class library for controlling the ES9018 Sabre32 DAC from an Arduino via I2C 

  In essence, this library repackages much of the code from the HifiDuino Sabre32 project (see https://hifiduino.wordpress.com/sabre32/)
  into a C++ class in order to make it more 'user friendly' by abstracting the programmer from having to deal with the underlying 
  bits 'n' bytes of the DAC's registry settings.

  All DAC registry writes can be viewed via the Arduino's serial monitor

  (see the WIRE library for details on connecting an I2C device to an Arduino board. Be aware that most I2C devices use 3.3 volts!)
