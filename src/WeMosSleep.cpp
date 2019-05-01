//=========================================================================================
//WeMosSleep.cpp Rev0
// This C++ file implements a class, WeMosSleep.
// created by Joe Barbetta Mar 2019
//
// see file comments in WeMosSleep.h for more information

#include "WeMosSleep.h"  //include header file


//This is the class constructor (note that it has the same name as the class) and
// its code is invoked when a class instance (object) is created.
//
WeMosSleep::WeMosSleep(){
  _sleepEventCtr = 0;
  //set parameters to defaults, note that they can be changed by the user later on
  _napSecs = _NAP_SECS_DFLT;      //12
  _sleepMins = _SLEEP_MINS_DFLT;  //15
}


//The setNapSeconds method sets the number of seconds the WeMos board will take a short
// nap for. These short naps are needed to keep the E122 power bank output on.  A load
// of more than 35mA must be applied to the power bank at a period of no more than 15
// seconds to keep its output on.  If the power bank detects a no load comdition for
// about 16 seconds, if will turn its output off to conserve power by turning its
// step-up converter off.
//If this method is not called a default value of 12 is used.
// the variable _napSecs is used in calls to ESP.deepSleep().  
void WeMosSleep::setNapSeconds(uint32_t seconds){
  _napSecs = seconds;
  if(_napSecs < 1) _napSecs = 1;
  if(_napSecs > 60) _napSecs = 60;
}


//The setSleepMinutes method sets the period (in minutes) in which the WeMos will
// publish data.  During this period the WeMos is taking short naps.
//If this method is not called a default value of 15 is used.
//
void WeMosSleep::setSleepMinutes(uint32_t minutes){
  _sleepMins = minutes;
  if(_sleepMins < 1) _sleepMins = 1;              //limit to 1 minute
  if(_sleepMins > 60 * 24) _sleepMins = 60 * 24;  //limit to 24 hours
}


//This method should be called from setup() before WiFi connect.
//Any code following this call, such as WiFi and MQTT connecting, will only be invoked
// if the Sleep period is finished.  Note that code will also be invoked after the first
// call of this function to allow testing of the WiFi/MQTT connecting and publishing
// code, without having to wait for a sleep period to pass.
void WeMosSleep::checkWake(){

  rst_info *resetInfo; //need to define as pointer for ESP.getResetInfoPtr()

  resetInfo = ESP.getResetInfoPtr();  //get reason for reset
  // 5 = reset button or wake from deep sleep
  // 6 = power-up or reset after upload
  Serial.print("Reset Info="); Serial.println((*resetInfo).reason);

  if((*resetInfo).reason == 5){  //if wake from deep sleep
    Serial.println("Wake from Deep Sleep");
    //read _sleepEventCtr from nonvolatile memory
    ESP.rtcUserMemoryRead(0, &_sleepEventCtr, sizeof(_sleepEventCtr));
    _sleepEventCtr++;
    //write _sleepEventCtr to nonvolatile memory
    ESP.rtcUserMemoryWrite(0, &_sleepEventCtr, sizeof(_sleepEventCtr));
    Serial.print("sleepCtr=");Serial.println(_sleepEventCtr);
    
    if(_sleepEventCtr * _napSecs > _sleepMins * 60){
      _sleepEventCtr = 0;  //reset counter
      //write _sleepEventCtr to nonvolatile memory
      ESP.rtcUserMemoryWrite(0, &_sleepEventCtr, sizeof(_sleepEventCtr));
      Serial.println("Time to Publish");

    }else{
      delay(500);  //insure power bank stays awake
      Serial.println("Take short nap\n");
      ESP.deepSleep(_napSecs * 1000000);
      //note that execution will be halted at deepSleep calls

    }


  }else{                        //if power up
    Serial.println("Power Up");
    _sleepEventCtr = 0;
    //write _sleepEventCtr to nonvolatile memory
    ESP.rtcUserMemoryWrite(0, &_sleepEventCtr, sizeof(_sleepEventCtr));
    //will continue on to publishing
  }
}


//This method should be called at end of loop().
//
void WeMosSleep::sleep(){
  //this function accepts an arugument in microseconds, hence the * 1000000
  ESP.deepSleep(_napSecs * 1000000);
}
