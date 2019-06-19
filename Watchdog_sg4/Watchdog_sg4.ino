#include <WatchDogTimers.h>

//Pin Assignments--------------------------------------------------
#define navi 6
#define sensor 7
#define router 2
#define led 3
//#define motorb 2
//#define powerb 3

#define sensorOut A1
#define naviOut A0
#define routerOut A4
#define ledOut A2

//#define override (whatever pin)

int pinToReboot; //Variable containing the desired card/pin number----

//Setting Time values----------------------------------------------------------------------------------------------------------
#define COMPLETE_BOOT_TIME 45000 //Time given to boot up the entire robot----
#define NAVI_BOOT_TIME 6500 //Time Given to boot up Navi/Sensor ----
#define OTHER_BOOT_TIME 4000 //Time given to boot up a pin not already listed----
#define LOW_TIME 2000 //Time to hold pins low while resetting----
#define WAIT_TIME 5000 //Time to wait until it is determmined that a processor is hanging----

//Creating the timers and giving them their values-----------------------------------------------------------------------------
Timers sensorTimer(WAIT_TIME);
Timers naviTimer(WAIT_TIME); 
Timers routerTimer(WAIT_TIME);
Timers ledTimer(WAIT_TIME);
Timers rebootTimer(COMPLETE_BOOT_TIME);
Timers pinBootTimer();
Timers lowTimer(LOW_TIME);

//Creating initial/old value trackers for the processors-----------------------------------------------------------------------
bool oldValueSensor;
bool oldValueNavi;
bool oldValueRouter;
bool oldValueLed;
bool mustRebootPin;//Is true if we must reboot
bool fullReboot;//Is true if pins are low waiting to be pulled HIGH
bool bootWaiting;//Is true if we are waiting for a reboot
bool allPinsLow;//Is true if all Pins need to be LOW
bool allPinsHigh;//Is true if all Pins need to be HIGH

//Setup for the Watchdog-------------------------------------------------------------------------------------------------------
void setup() 
{
  //Setting the INPUT pins-----------------------------------------------------------------------------------------------------
  pinMode(sensor, INPUT);
  pinMode(navi, INPUT);
  pinMode(router, INPUT);
  pinMode(led, INPUT);

  //Setting the OUTPUT pins----------------------------------------------------------------------------------------------------
  pinMode(sensorOut, OUTPUT);
  pinMode(naviOut, OUTPUT);
  pinMode(routerOut, OUTPUT);
  pinMode(ledOut, OUTPUT);

  //Setting the OUTPUT pins high-----------------------------------------------------------------------------------------------
  digitalWrite(sensorOut, HIGH);
  digitalWrite(naviOut, HIGH);
  digitalWrite(routerOut, HIGH);
  digitalWrite(ledOut, HIGH);

  //Reading the inital/old input values-----------------------------------------------------------------------------------------
  oldValueSensor = digitalRead(sensor);
  oldValueNavi = digitalRead(navi);
  oldValueRouter = digitalRead(router);
  oldValueLed = digitalRead(led);
  
  //Reseting all the timers because they would have expired while waiting on boot timer------------------------------
  sensorTimer.resetTimer();
  naviTimer.resetTimer();
  routerTimer.resetTimer();
  ledTimer.resetTimer();
  rebootTimer.resetTimer();
  
  //Resetting variables ------------------------------------------------------------------------------------------
  bootWaiting = true;
  fullReboot = false;
  mustRebootPin = false;
  allPinsLow = false;
  allPinsHigh = false;

  while(1);
}

void loop() 
{
  //while(1);
  //Checking to see if any processor is stalling------------------------------------------------------
  if(!mustRebootPin && !fullReboot && !bootWaiting)
  {
    checkHanging(&sensorTimer, sensor, sensorOut, &oldValueSensor); //Checking the Sensor----
  }
  if(!mustRebootPin && !fullReboot && !bootWaiting)
  {
    checkHanging(&naviTimer, navi, naviOut, &oldValueNavi); //Checking the Navigation processor----
  }
  if(!mustRebootPin && !fullReboot && !bootWaiting)
  {
    checkHanging(&routerTimer, router, routerOut, &oldValueRouter); //Checking the Router----
  }
  if(!mustRebootPin && !fullReboot && !bootWaiting)
  {
    checkHanging(&ledTimer, led, ledOut, &oldValueLed); //Checking the LEDs----
  }
  
//Resolving Pin Changes ----------------------------------------------------
  //A pin has been identified to be stalling, mustrebootpin == TRUE----
  if(mustRebootPin) 
  {
   //Pin # is sent through pinReboot----
   pinReboot(pinToReboot);
   //mustRebootPin is reset to False----
   mustRebootPin = false;
  }

  //------------------------------------------------------------------------------------------------------------------------
  if(bootWaiting) //bootWaiting -> robot is waiting for reboot to complete-----------------------
  {
    if(rebootTimer.timerDone())//Once the Reboot timer is done-----
    {
      rebootTimer.resetTimer();//Reset reboot timer---
      bootWaiting = false; //set bootWaiting to false, saying no the robot isn't waiting anymore---

      //Take the 'old'/input values of the cards. -----------------------------------------
      oldValueSensor = digitalRead(sensor); 
      oldValueNavi = digitalRead(navi);
      oldValueRouter = digitalRead(router);
      oldValueLed = digitalRead(led);

      //Reset all timers because they would have expired while rebooting the robot --------------------------------------------
      sensorTimer.resetTimer();
      naviTimer.resetTimer();
      routerTimer.resetTimer();
      ledTimer.resetTimer();
    }
  }

//------------------------------------------------------------------------------------------------------------------
//    
 
  //The Master Reboot has been triggered-----------------
  if(fullReboot) 
  {
    //Begin the rebooting phase by setting all pins low----
    setAllLow();

    //If we have gone through the Master Reboot
    if(lowTimer.timerDone()) //^^^^ AND lowTimer has Expired. -----
    {
      lowTimer.resetTimer(); //THEN Reset lowTimer, ----
      fullReboot = false; //Set fullReboot as FALSE ----
      if(allPinsHigh) //^^^^ AND IF allPinsHigh is TRUE ----
      {
        setAllHigh(); //Run setAllHigh()---
      }
      else //IF allpinshigh is not true after the master reboot, then there is an error and we need to reboot again. 
      {
        setAllLow(); //Reboot Pin---
      }
    }
  }
}

void checkHanging(Timers * timer, int inPin, int outPin, bool *oldValue)//------------------------------------------------------
{
  //If the current state if the inPin equals the previous state of the inPin---------------------------------
  if(digitalRead(inPin) == *oldValue) 
  {
    if(timer->timerDone())//When the timer expires-----
    {
      mustRebootPin = true;
      pinToReboot = outPin;
    }
  }
  else 
  {
    //The state of the inPin has changed so reset the time-----
    timer->resetTimer();
    *oldValue = digitalRead(inPin);
  }
  return;
}


void pinReboot(int pin)//Rebooting Pins/cards that need to be rebooted-------------------------------------------------------------
{
  int rebootTime;//Variable to store time for rebooting the cards----

  //Determine how long to wait for reboot before watchdog begins checking again------------------
  if(pin == routerOut) //Router Reboot Timer------
  {
    rebootTime = COMPLETE_BOOT_TIME;
  }
  else if(pin == naviOut) //Navigational Reboot Timer --------
  {
    rebootTime = NAVI_BOOT_TIME;
  }
  else //Other Reboot Timer------------
  {
    rebootTime = OTHER_BOOT_TIME;
  }
  //Set found pin to be LOW
  digitalWrite(pin, LOW);
  //Wait a set amount of time to reset
  lowTimer.resetTimer();
  while(!lowTimer.timerDone()) {}

  digitalWrite(pin, HIGH);//Set the rebooting card/pin HIGH,----- 
  rebootTimer.setInterval(rebootTime);//Allocate the proper amount of time to reboot-----
  rebootTimer.resetTimer();//Reset the reboot timer-----
  bootWaiting = true;//Start signaling that we are waiting to finish rebooting-----

  //Reset all inital/old values for the processors-------------------------------------------------
  oldValueSensor = digitalRead(sensor);
  oldValueNavi = digitalRead(navi);
  oldValueRouter = digitalRead(router);
  oldValueLed = digitalRead(led);

   //Reset all timers because they would have expired while rebooting the robot --------------------------------------------
   sensorTimer.resetTimer();
   naviTimer.resetTimer();
   routerTimer.resetTimer();
   ledTimer.resetTimer();

   return;
}

//Function to set all Pins LOW-------------------------------------------------
void setAllLow()
{
  digitalWrite(sensorOut, LOW);
  digitalWrite(naviOut, LOW);
  digitalWrite(routerOut, LOW);
  digitalWrite(ledOut, LOW);
  lowTimer.resetTimer();
  allPinsHigh = true;
  return;
}

//Function to set all Pins HIGH------------------------------------------------
void setAllHigh()
{
  digitalWrite(sensorOut, HIGH);
  digitalWrite(naviOut, HIGH);
  digitalWrite(routerOut, HIGH);
  digitalWrite(ledOut, HIGH);
  rebootTimer.setInterval(COMPLETE_BOOT_TIME);
  rebootTimer.resetTimer();
  bootWaiting = true;
  allPinsHigh = false;
  return;
}

