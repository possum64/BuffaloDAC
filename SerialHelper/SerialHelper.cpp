#include "SerialHelper.h"

#ifdef UseRemoteDebug
  RemoteDebug Debug;
#endif
void Msg::begin(String &hostname, Msg::Level level)
{
#ifdef UseRemoteDebug
   switch (level)
    {
        case D:
           Debug.begin(hostname, DEBUG);
           break;
        case I:
           Debug.begin(hostname, INFO);
           break;
       case W:
           Debug.begin(hostname, WARNING);
           break;
        case E:
           Debug.begin(hostname, ERROR);
           break;
        default:
           Debug.begin(hostname);
    };
    Debug.setResetCmdEnabled(true);
    Debug.showColors(true);
#endif
}

   
void Msg::loop()
{
  debugHandle();
};
void Msg::setLevel(Msg::Level level)
{
#ifndef UseRemoteDebug
   switch (level)
    {
        case D:
           debugSetLevel(4);
           break;
        case I:
           debugSetLevel(3);
           break;
       case W:
           debugSetLevel(2);
           break;
        case E:
           debugSetLevel(1);
           break;
    };
#endif
};

void Msg::print(Msg::Level level, const char val[])
{
    #ifdef UseRemoteDebug
    switch (level)
    {
        case D:
        rprintD(val);
        break;
        case I:
        rprintI(val);
        break;
        case W:
        rprintW(val);
        break;
        case E:
        rprintE(val);
        break;
    };
    #ifdef UseSerial
        Serial.print(val);
    #endif
    #else
    switch (level)
    {
        case D:
        printD(val);
        break;
        case I:
        printI(val);
        break;
        case W:
        printW(val);
        break;
        case E:
        printE(val);
        break;
    };
    #endif;
};

void Msg::print(Msg::Level level, char val)
{
    #ifdef UseRemoteDebug
    switch (level)
    {
        case D:
        rprintD(val);
        break;
        case I:
        rprintI(val);
        break;
        case W:
        rprintW(val);
        break;
        case E:
        rprintE(val);
        break;
    };
    #ifdef UseSerial
        Serial.print(val);
    #endif
    #else
    switch (level)
    {
        case D:
        printD(val);
        break;
        case I:
        printI(val);
        break;
        case W:
        printW(val);
        break;
        case E:
        printE(val);
        break;
    };
    #endif;
};

void Msg::print(Msg::Level level, unsigned char val, int base)
{
    Msg::print(level, (long) val, base);
};

void Msg::print(Msg::Level level, int val, int base)
{
  return Msg::print(level, (long) val, base);
};

void Msg::print(Msg::Level level, unsigned int val, int base)
{
  return Msg::print(level, (unsigned long) val, base);
}

void Msg::print(Msg::Level level, long val, int base)
{
    #ifdef UseRemoteDebug
    switch (level)
    {
        case D:
        rprintD(val);
        break;
        case I:
        rprintI(val);
        break;
        case W:
        rprintW(val);
        break;
        case E:
        rprintE(val);
        break;
    };
    #ifdef UseSerial
        Serial.print(val);
    #endif
    #else
    switch (level)
    {
        case D:
        printD(val);
        break;
        case I:
        printI(val);
        break;
        case W:
        printW(val);
        break;
        case E:
        printE(val);
        break;
    };
    #endif;
};

void Msg::print(Msg::Level level, unsigned long val, int base)
{
    #ifdef UseRemoteDebug
    switch (level)
    {
        case D:
        rprintD(val);
        break;
        case I:
        rprintI(val);
        break;
        case W:
        rprintW(val);
        break;
        case E:
        rprintE(val);
        break;
    };
    #ifdef UseSerial
        Serial.print(val);
    #endif
    #else
    switch (level)
    {
        case D:
        printD(val);
        break;
        case I:
        printI(val);
        break;
        case W:
        printW(val);
        break;
        case E:
        printE(val);
        break;
    };
    #endif;
};

void Msg::print(Msg::Level level, const __FlashStringHelper* val)
{
    #ifdef UseRemoteDebug
    switch (level)
    {
        case D:
        rprintD(val);
        break;
        case I:
        rprintI(val);
        break;
        case W:
        rprintW(val);
        break;
        case E:
        rprintE(val);
        break;
    };
    #ifdef UseSerial
        Serial.print(val);
    #endif
    #else
    switch (level)
    {
        case D:
        printD(val);
        break;
        case I:
        printI(val);
        break;
        case W:
        printW(val);
        break;
        case E:
        printE(val);
        break;
    };
    #endif;
};

void Msg::print(Msg::Level level, String val)
{
    #ifdef UseRemoteDebug
    switch (level)
    {
        case D:
        rprintD(val);
        break;
        case I:
        rprintI(val);
        break;
        case W:
        rprintW(val);
        break;
        case E:
        rprintE(val);
        break;
    };
    #ifdef UseSerial
        Serial.print(val);
    #endif
    #else
    switch (level)
    {
        case D:
        printD(val);
        break;
        case I:
        printI(val);
        break;
        case W:
        printW(val);
        break;
        case E:
        printE(val);
        break;
    };
    #endif;
};

void Msg::print(const char val[])
{
    Msg::print(defaultLevel, val);
};

void Msg::print(char val)
{
    Msg::print(defaultLevel, val);
};

void Msg::print(unsigned char val, int base)
{
    Msg::print(defaultLevel, val, base);
};

void Msg::print(int val, int base)
{
    Msg::print(defaultLevel, val, base);
};

void Msg::print(unsigned int val, int base)
{
    Msg::print(defaultLevel, val, base);
};

void Msg::print(long val, int base)
{
    Msg::print(defaultLevel, val, base);
};

void Msg::print(unsigned long val, int base)
{
    Msg::print(defaultLevel, val, base);
};

void Msg::print(double val, int base)
{
    Msg::print(defaultLevel, val, base);
};

void Msg::print(String val)
{
    Msg::print(defaultLevel, val);
};

void Msg::print(const __FlashStringHelper* val)
{
    Msg::print(defaultLevel, val);
};


// Print Line
void Msg::println()
{
    println(' ');
}

void Msg::println(Msg::Level level, const char val[])
{
    #ifdef UseRemoteDebug
    switch (level)
    {
        case D:
        rprintDln(val);
        break;
        case I:
        rprintIln(val);
        break;
        case W:
        rprintWln(val);
        break;
        case E:
        rprintEln(val);
        break;
    };
    #ifdef UseSerial
        Serial.println(val);
    #endif
    #else
    switch (level)
    {
        case D:
        printlnD(val);
        break;
        case I:
        printlnI(val);
        break;
        case W:
        printlnW(val);
        break;
        case E:
        printlnE(val);
        break;
    };
    #endif;
};

void Msg::println(Msg::Level level, char val)
{
    #ifdef UseRemoteDebug
    switch (level)
    {
        case D:
        rprintDln(val);
        break;
        case I:
        rprintIln(val);
        break;
        case W:
        rprintWln(val);
        break;
        case E:
        rprintEln(val);
        break;
    };
    #ifdef UseSerial
        Serial.println(val);
    #endif
    #else
    switch (level)
    {
        case D:
        printlnD(val);
        break;
        case I:
        printlnI(val);
        break;
        case W:
        printlnW(val);
        break;
        case E:
        printlnE(val);
        break;
    };
    #endif;
};

void Msg::println(Msg::Level level, unsigned char val, int base)
{
    Msg::println(level, (long) val, base);
};

void Msg::println(Msg::Level level, int val, int base)
{
  return Msg::println(level, (long) val, base);
};

void Msg::println(Msg::Level level, unsigned int val, int base)
{
  return Msg::println(level, (unsigned long) val, base);
}

void Msg::println(Msg::Level level, long val, int base)
{
    #ifdef UseRemoteDebug
    switch (level)
    {
        case D:
        rprintDln(val);
        break;
        case I:
        rprintIln(val);
        break;
        case W:
        rprintWln(val);
        break;
        case E:
        rprintEln(val);
        break;
    };
    #ifdef UseSerial
        Serial.println(val);
    #endif
    #else
    switch (level)
    {
        case D:
        printlnD(val);
        break;
        case I:
        printlnI(val);
        break;
        case W:
        printlnW(val);
        break;
        case E:
        printlnE(val);
        break;
    };
    #endif;
};

void Msg::println(Msg::Level level, unsigned long val, int base)
{
    #ifdef UseRemoteDebug
    switch (level)
    {
        case D:
        rprintDln(val);
        break;
        case I:
        rprintIln(val);
        break;
        case W:
        rprintWln(val);
        break;
        case E:
        rprintEln(val);
        break;
    };
    #ifdef UseSerial
        Serial.println(val);
    #endif
    #else
    switch (level)
    {
        case D:
        printlnD(val);
        break;
        case I:
        printlnI(val);
        break;
        case W:
        printlnW(val);
        break;
        case E:
        printlnE(val);
        break;
    };
    #endif;
};

void Msg::println(Msg::Level level, const __FlashStringHelper* val)
{
     #ifdef UseRemoteDebug
    switch (level)
    {
        case D:
        rprintDln(val);
        break;
        case I:
        rprintIln(val);
        break;
        case W:
        rprintWln(val);
        break;
        case E:
        rprintEln(val);
        break;
    };
    #ifdef UseSerial
        Serial.println(val);
    #endif
    #else
    switch (level)
    {
        case D:
        printlnD(val);
        break;
        case I:
        printlnI(val);
        break;
        case W:
        printlnW(val);
        break;
        case E:
        printlnE(val);
        break;
    };
    #endif;
};

void Msg::println(Msg::Level level, String val)
{
     #ifdef UseRemoteDebug
    switch (level)
    {
        case D:
        rprintDln(val);
        break;
        case I:
        rprintIln(val);
        break;
        case W:
        rprintWln(val);
        break;
        case E:
        rprintEln(val);
        break;
    };
    #ifdef UseSerial
        Serial.println(val);
    #endif
    #else
    switch (level)
    {
        case D:
        printlnD(val);
        break;
        case I:
        printlnI(val);
        break;
        case W:
        printlnW(val);
        break;
        case E:
        printlnE(val);
        break;
    };
    #endif;
};

void Msg::println(const char val[])
{
    Msg::println(defaultLevel, val);
};

void Msg::println(char val)
{
    Msg::println(defaultLevel, val);
};

void Msg::println(unsigned char val, int base)
{
    Msg::println(defaultLevel, val, base);
};

void Msg::println(int val, int base)
{
    Msg::println(defaultLevel, val, base);
};

void Msg::println(unsigned int val, int base)
{
    Msg::println(defaultLevel, val, base);
};

void Msg::println(long val, int base)
{
    Msg::println(defaultLevel, val, base);
};

void Msg::println(unsigned long val, int base)
{
    Msg::println(defaultLevel, val, base);
};

void Msg::println(double val, int base)
{
    Msg::println(defaultLevel, val, base);
};

void Msg::println(String val)
{
    Msg::println(defaultLevel, val);
};

void Msg::println(const __FlashStringHelper* val)
{
    Msg::println(defaultLevel, val);
};
