/*
 * Timers.h
 *
 * Created: 4/28/2018 3:13:28 PM
 *  Author: John
 */ 
#ifndef TIMERS_H
#define TIMERS_H
#include "Definitions.h"

#include <avr/io.h>				/* Standard I/O Library*/
#include <stdbool.h>			/* Standard Bool library*/
#include <avr/interrupt.h>		/* Standard Timer/Interrupt Library*/
#define WAIT_TIME 450

//int cardTimer1;
//int cardTimer2;
//int cardTimer3;
//int rebootTime;

bool resettingCARD1;
bool resettingCARD2;
bool resettingCARD3;

typedef struct{
	unsigned long timerInterval;
	unsigned long lastMillis;
}timer_t;

bool timerDone(timer_t * t);
bool timerDoneNoReset(timer_t * t);
void setTimerInterval(timer_t * t, unsigned long interval);
void resetTimer(timer_t * t);
void globalTimerTracker( );
unsigned long millis(void);


void initTimers(void);
int resetTimers(int constantTime);
int checkTimers(int time);
//*****************************************
//**********    END OF H file  ************
//*****************************************

#endif