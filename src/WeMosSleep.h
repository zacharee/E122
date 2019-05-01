//=========================================================================================
//WeMosSleep.h Rev0
// This header file defines 1 class, WeMosSleep.
// created by Joe Barbetta Mar 2019 for Stevens Wemos temperature, humidity and light
//  sensing logging platform
//
// This code manages the sleep mode of the WeMos ESP8266 with the Design Lab powerbank.
//The powerbank shuts off if its load falls below about 35mA for more than 16 seconds,
//so the WeMos cannot be in a deep sleep mode for more than 16 seconds.  To accomodate
//this the WeMos will take deep sleep "naps" of only 12 seconds.  After each nap the
//WeMos will powerup for about half a second to keep the powerbank on.  A nap counter
//is maintained so that after a number of nap periods that equals the desired publish
//period, i.e. 15 minutes, the WeMos will connect to WiFi and the MQTT server and publish
//before napping again.
// A normal variable cannot be used for the nap counter because during the deep sleep mode
//used for naps the ESP8266 RAM is not maintained so special non-volatile RAM is used.
//This non-volatile RAM is called RTC (Real Time Clock) RAM and special functions are used
//to read from and write to this RAM.
// Note that the deep sleep mode current of the ESP8266 is microamps, however, because
//the WeMos has an on-board USB interface and 3.3V regulator, the WeMos board consumes
//about 10mA in deep sleep mode.

//using WeMosSleep library in Arduino sketch:
//
//#include "WeMosSleep.h"  //include sleep function library
//
//WeMosSleep sleep;  //create class instance for sleep functions
//
//calls placed in setup() after Serial.println("Wemos POWERING UP ......... ");
//  sleep.setNapSeconds(12);
//  sleep.setSleepMinutes(15);
//  //if it is time to wake code after this call will run, otherwise chip will go
//  // to sleep and thus code after this call will Not run
//  sleep.checkWake();
//
//call placed at end of loop()
//  sleep.sleep();  //this will stop loop

#ifndef WeMosSleep_h  //this is an inclusion guard to insure this header file is not
#define WeMosSleep_h  // included multiple times, which would cause compile errors

#include "Arduino.h"
#include <ESP8266WiFi.h>

class WeMosSleep  //class for sleep functions
{
  public:
    WeMosSleep();  //constructor
    void setNapSeconds(uint32_t seconds);
    void setSleepMinutes(uint32_t minutes);
    void checkWake();
    void sleep();

  private:
    uint32_t _sleepEventCtr;
    uint32_t _napSecs;
    uint32_t _sleepMins;

    const uint32_t _NAP_SECS_DFLT = 12;    //default value
    const uint32_t _SLEEP_MINS_DFLT = 15;  //default value
};

#endif  //end of inclusion guard
