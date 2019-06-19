/* WatchDog.c
 *
 * Created: 10/17/2017 5:12:10 PM
 * Author : Scott Grisso
 * Last Modified: 5/4/18 3:38 PM
 */ 

	/* #include Libraries */
#include <avr/io.h>				/* Standard I/O Library*/
#include <stdbool.h>			/* Standard Bool library*/
#include "Timers.h"

#include "Definitions.h"

	/* Logic variables*/
bool oldValueCARD1;
bool oldValueCARD2;
bool oldValueCARD3;
int mustRebootPin;
int fullReboot;
int bootWaiting;

	/* Data Storage Variables*/
char *pinToReboot;

	/*Prototype Functions */
void checkHanging(char, char, bool*, timer_t *statusTimer);	/*Function to check each processor to see if they are stalling*/
void pinReboot(char);				/*Function to restart the pin that is found to be stalling*/
void testToggle();
void getNewReadings();
void restoreCard();

timer_t ResetTimer,cardTimer1,cardTimer2,cardTimer3;
timer_t BootTimer[3];
int main()
{
	DDRD = 0x00;//0b11111111;		/*Set Input Pins */
	DDRC = 0xFF;//0b00000000;		/*Set Output Pins*/
	PORTC = 0xFF;					/*Set output HIGH*/
	//PORTC = 0x00;				/*Set Output Low (Allow Power to Pass)*/
	
	//-------------------------------
	/*PIN 13 LED*/
	DDRB = 1 << PORTB5;
	PORTB = 1<<PORTB5;
	//----------------------------------
		/*Initialize the timers*/
	initTimers();
		
	//Global Timer Interrupt enable-----
	sei();	
	
	//----------------------------------
		/*Initialize variables*/
	fullReboot = 0;
	mustRebootPin = 0;
	TIMSK0 = 0 << OCIE0A;
	
	//Resetting the timer overflow
	TCNT1 = 0;

	/*Get initial Readings*/ 
	getNewReadings();

	/*Set the inital timers*/
	setTimerInterval(&BootTimer[0],COMPLETE_BOOT_TIME);
	setTimerInterval(&BootTimer[1],COMPLETE_BOOT_TIME);
	setTimerInterval(&BootTimer[2],COMPLETE_BOOT_TIME);
	setTimerInterval(&cardTimer1,TIME_INACTIVE);
	setTimerInterval(&cardTimer2,TIME_INACTIVE);
	setTimerInterval(&cardTimer3,TIME_INACTIVE);
	setTimerInterval(&ResetTimer, HIGH_TIME);

		/*Main Loop*/
    while (1) 
    {
		#ifdef CARD1
		if(timerDoneNoReset(&BootTimer[0]))
		{
			checkHanging(CARD1, CARD1Out, &oldValueCARD1, &cardTimer1);
			if(mustRebootPin == 1)
			{
				pinReboot(CARD1Out);
				mustRebootPin = 0;

				oldValueCARD1 = PIND & (1 << CARD1);
				
				/*Set cardTimer1's wait_time*/
				setTimerInterval(&cardTimer1, WAIT_TIME);
			}
		}
		else
		{
			resetTimer(&cardTimer1);
		}
		#endif

		#ifdef CARD2

		if(timerDoneNoReset(&BootTimer[1]))
		{
			checkHanging(CARD2, CARD2Out, &oldValueCARD2, &cardTimer2);
			if(mustRebootPin == 1)
			{
				pinReboot(CARD2Out);
				mustRebootPin = 0;

				oldValueCARD2 = PIND & (1 << CARD2);
			
				/*Set cardTimer2's wait_time*/
				setTimerInterval(&cardTimer2, WAIT_TIME);
			}
		}
		else
		{
			resetTimer(&cardTimer2);
		}
		#endif

		#ifdef CARD3
		if(timerDoneNoReset(&BootTimer[2]))
		{
			checkHanging(CARD3, CARD3Out, &oldValueCARD3, &cardTimer3);
			if(mustRebootPin == 1)
			{
				pinReboot(CARD3Out);
				mustRebootPin = 0;

				oldValueCARD3 = PIND & (1 << CARD3);
			
				/*Set cardTimer3's wait_time*/
				setTimerInterval(&cardTimer3, WAIT_TIME);
			}
		}
		else
		{
			resetTimer(&cardTimer3);
		}
		#endif

		restoreCard();
    }
}

void checkHanging(char inPin, char outPin, bool* oldValue, timer_t *statusTimer)	/*Function to check if a pin is stalling*/
{
	/*	Pre-condition: Takes the inputPin and outputPin for each processor and compares them.	*/
	/*	Post-condition: Returns if the selected pins are stalling or not.						*/
	bool newValue = PIND & (1 << inPin );
	if (newValue != *oldValue)						/*If the Values are different...*/
	{
		/*Set Pin 13 LED high*/
		PORTB = (1<<PORTB5);

		/*Setting the pin High*/
		PORTC = PORTC |(1 << outPin);

		/*Storing the current state*/
		*oldValue = newValue;
		/*Resetting the inactive timer*/
		resetTimer(statusTimer);
	}	
	else if(timerDone(statusTimer))		/*If Pin Values are the same...Check to see if the timer has run out. If it has, then the pin is stalling and needs to be reset*/
	{
		PORTB = (0<<PORTB5);
		mustRebootPin = 1;		/*Trip Flag*/
		pinToReboot = &outPin;		/*Store the pin number*/
	}		
	return;
}

void pinReboot(char pin)
{
	/*	Pre-Condition: Takes in the Pin determined to be stalling					*/
	/*	Post-Condition: Reboots pin and returns bootWaiting to be true				*/
	switch(pin)
	{
		case  CARD1Out:
		{
			resettingCARD1 = true;
			PORTC = PORTC & ~(1 << pin);				/*Set Pin Low*/
			resetTimer(&BootTimer[0]);
			break;
		}
		case  CARD2Out:
		{
			resettingCARD2 = true;
			PORTC = PORTC & ~(1 << pin);				/*Set Pin Low*/
			resetTimer(&BootTimer[1]);
			
			break;
		}
		case  CARD3Out:
		{
			resettingCARD3 = true;
			PORTC = PORTC & ~(1 << pin);				/*Set Pin Low*/
			resetTimer(&BootTimer[2]);
			
			break;
		}
		default:	//should never reach this
		break;
	}
	resetTimer(&ResetTimer);	//naming needs changed
}

void restoreCard()
{
	if(timerDone(&ResetTimer))
	{
		if(resettingCARD1)
		{
			PORTC = PORTC |(1 << CARD1Out);
			resettingCARD1 = false;
		}

		if(resettingCARD2)
		{
			PORTC = PORTC |(1 << CARD2Out);
			resettingCARD2 = false;
		}

		if(resettingCARD3)
		{
			PORTC = PORTC |(1 << CARD3Out);
			resettingCARD3 = false;
		}
	}
	
}

void getNewReadings()
{
	#ifdef CARD1
	oldValueCARD1 = PIND & (1 << CARD1);
	#endif

	#ifdef CARD2
	oldValueCARD2 = PIND & (1 << CARD2);
	#endif

	#ifdef CARD3
	oldValueCARD3 = PIND & (1 << CARD3);
	#endif
}

void testToggle()
{
	int timer45;
	timer45 = TCNT1 + 10;
	while(TCNT1 <= timer45)
	{}
	PORTC = 0x00;
	timer45 = TCNT1 + 10;
	while(TCNT1 <= timer45)
	{}
	PORTC = 0xFF;
	return;
}
