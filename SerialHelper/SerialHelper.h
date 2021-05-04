#ifndef SERIALHELPER_h
#define SERIALHELPER_h

#include "global.h"

#ifdef UseRemoteDebug
  #include "RemoteDebug.h" //https://github.com/JoaoLopesF/RemoteDebug
#else
  #include "SerialDebug.h" //https://github.com/JoaoLopesF/SerialDebug
#endif


class Msg
{
  public:
    enum Level{D, I, W, E}; // Debug, Information, Warning, Error
    static const Msg::Level defaultLevel = I;

    static void begin(String &hostname, Msg::Level level = Msg::I);
    static void loop();
    static void setLevel(Level level);

    // Print methods
    static void print(Level level, const char[]);
    static void print(Level level, char);
    static void print(Level level, unsigned char, int = DEC);
    static void print(Level level, int, int = DEC);
    static void print(Level level, unsigned int, int = DEC);
    static void print(Level level, long, int = DEC);
    static void print(Level level, unsigned long, int = DEC);
    static void print(Level level, double, int = 2);
    static void print(Level level, String val);
    static void print(Level level, const __FlashStringHelper* val);
    static void print(const char[]);
    static void print(char);
    static void print(unsigned char, int = DEC);
    static void print(int, int = DEC);
    static void print(unsigned int, int = DEC);
    static void print(long, int = DEC);
    static void print(unsigned long, int = DEC);
    static void print(double, int = 2);
    static void print(String val);
    static void print(const __FlashStringHelper* val);

    // Print Line methods
    static void println(Level level, const char[]);
    static void println(Level level, char);
    static void println(Level level, unsigned char, int = DEC);
    static void println(Level level, int, int = DEC);
    static void println(Level level, unsigned int, int = DEC);
    static void println(Level level, long, int = DEC);
    static void println(Level level, unsigned long, int = DEC);
    static void println(Level level, double, int = 2);
    static void println(Level level, String val);
    static void println(Level level, const __FlashStringHelper* val);
    static void println();
    static void println(const char[]);
    static void println(char);
    static void println(unsigned char, int = DEC);
    static void println(int, int = DEC);
    static void println(unsigned int, int = DEC);
    static void println(long, int = DEC);
    static void println(unsigned long, int = DEC);
    static void println(double, int = 2);
    static void println(String val);
    static void println(const __FlashStringHelper* val);
  private:
  	static const uint8_t PROFILER = 0; 	// Used for show time of execution of pieces of code(profiler)
    static const uint8_t VERBOSE = 1; 	// Used for show verboses messages
    static const uint8_t DEBUG = 2;   	// Used for show debug messages
    static const uint8_t INFO = 3;		// Used for show info messages
    static const uint8_t WARNING = 4;	// Used for show warning messages
    static const uint8_t ERROR = 5;		// Used for show error messages
    static const uint8_t ANY = 6;		// Used for show always messages, for any current debug level
};

#endif