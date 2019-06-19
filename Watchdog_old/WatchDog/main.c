/* WatchDog.c
 *
 * Created: 10/17/2017 5:12:10 PM
 * Author : Scott Grisso
 * Last Modified: 3/6/18 5:42 PM
		::Fixed issue with bit setting for comparisons  
 */ 

	/* #include Libraries */
//#include <WatchDogTimers.h>
#include <avr/io.h>				/* Standard I/O Library*/
#include <stdbool.h>			/* Standard Bool library*/
#include <avr/interrupt.h>		/* Standard Timer/Interrupt Library*/

		/* Define Time Constants */
	/* Old Arduino Clock cycles 
	#define ARDUINO_COMPLETE_BOOT_TIME 45000
	#define ARDUINO_NAVI_BOOT_TIME 6500
	#define ARDUINO_OTHER_BOOT_TIME 4000
	#define ARDUINO_LOW_TIME 2000
	#define ARDUINO_WAIT_TIME 5000
	*/
 
		/* Timer Definitions */		/* Definitions used for the ATMEGA 328p 16-bit timer/counter */
	#define BOTTOM 0x0000			/* BOTTOM = zero*/
	#define MAX 0xFFFF				/* MAX = maximum count possible*/
	#define TOP 0xFFFF				/* TOP = Maximum count before looping to BOTTOM*/
	//PRR.PRTIM1 = 0x00;				/* Enabling the 16-Bit timer, The TC1 bit in the Power */
									   /*Reduction Register must be set to zero tp enable TC1 */

/*New time constants*/
#define COMPLETE_BOOT_TIME 4500
#define NAVI_BOOT_TIME 650
#define OTHER_BOOT_TIME 400
#define LOW_TIME 200
#define WAIT_TIME 500

	/*Define Pins*/ 
/*	PORTD for INPUT
	PORTC for OUTPUT	*/
#define sensor PORTD3
#define sensorOut PORTC0
#define led PORTD4
#define ledOut PORTC1
#define router PORTD2
#define routerOut PORTC3

	/* Logic variables*/
bool oldValueSensor;
bool oldValueRouter;
bool oldValueLed;
bool mustRebootPin;
bool fullReboot;
bool bootWaiting;

int timer1;
int timer2;
int timer3;

int timer45;

	/* Data Storage Variables*/
int pinToReboot;
int rebootTime;

	/*Prototype Functions */
void initTimers();					/*Function to initialize the appropriate timers. Unsure if needed; probably just initialize in beginning of main()*/
bool checkHanging(char, char, bool, int);	/*Function to check each processor to see if they are stalling*/
void pinReboot(char);				/*Function to restart the pin that is found to be stalling*/

int main()
{
	DDRD = 0x00;//0b11111111;		/*Set Input Pins */
	DDRB = 0x20;
	DDRC = 0xFF;//0b00000000;		/*Set Output Pins*/
	PORTB = 0x20;
	PORTC = 0xFF;					/*Set output HIGH*/

		/*Loop to test Timers*/
		initTimers();
	

	//while(1)
	//{
		//timer45 = TCNT1 + 100;
		//while(TCNT1 <= timer45)  /* Approximately 1/10th of a second wait*/
		//{}					/* Wait until the TCNT1 Timer has exceeded the wait time*/
		//PORTC = 0x00;
		//timer45 = TCNT1 + 100;			/*Set timer back to 0, will start counting back up immediately */
		//while(TCNT1 <= timer45)
		//{}
		//PORTC = 0xFF;
	//}

		/*Get initial Readings*/ 
	oldValueSensor = PORTD & (1 << sensor);
	oldValueRouter = PORTD & (1 << router);
	oldValueLed = PORTD & (1 << led);

		/*Initialize variables*/
	bootWaiting = true;
	fullReboot = false;
	mustRebootPin = false;
	
	rebootTime = NAVI_BOOT_TIME;

	timer1 = TCNT1 + WAIT_TIME;
	timer2 = TCNT1 + WAIT_TIME;
	timer3 = TCNT1 + WAIT_TIME;

		/*Main Loop*/
    while (1) 
    {
		if (!mustRebootPin && !bootWaiting)		/*As long as there are no outstanding conditions, run checkHanging*/
		{
			oldValueSensor = checkHanging(sensor, sensorOut, oldValueSensor, timer1);	/*Check the Sensor Pin*/
		}
		if (!mustRebootPin && !bootWaiting)		
		{
			oldValueRouter = checkHanging(router, routerOut, oldValueRouter, timer2);	/*Check the Router Pin*/
		}
		if (!mustRebootPin && !bootWaiting)		
		{
			oldValueLed = checkHanging(led, ledOut, oldValueLed, timer3);				/*Check the LED Pin*/
		}

			/*Resolving outstanding conditions*/
		if (mustRebootPin)			/*If we need to reboot a pin*/
		{
			pinReboot(pinToReboot);		/*Call the pinReboot Program and pass the number of the pin that is stalling*/
			mustRebootPin = false;		/*Reset the Flag used to tell if we need to reboot a pin*/
		}

		if (bootWaiting)			/*If we just rebooted a pin and are waiting for the pin to fully reboot*/
		{
			int waitTimer = TCNT1 + rebootTime;
			if(waitTimer > 65000)
			{
				TCNT1 = 0;
				waitTimer = TCNT1 + rebootTime;
				timer1 = TCNT1 + WAIT_TIME;
				timer2 = TCNT1 + WAIT_TIME;
				timer3 = TCNT1 + WAIT_TIME;
			}
			while(TCNT1 < waitTimer)	/*Wait enough time for pin to fully reboot*/
			{}	
			bootWaiting = false;		/*Set bootWaiting flag to false*/
			
			oldValueSensor = sensor;	/*Reset 'old' values used for logic and comparison*/
			oldValueRouter = router;
			oldValueLed = led;
			
			TCNT1 = 0;					/*Reset Timer*/
			timer1 = TCNT1 + WAIT_TIME;
			timer2 = TCNT1 + WAIT_TIME;
			timer3 = TCNT1 + WAIT_TIME;
		}			
    }
}

void initTimers(void)
{
	/* Timer clock = I/O clock / 1024 */
	TCCR0B = (1<<CS02)|(1<<CS00);
	TCCR1B = (1<<CS12)|(1<<CS10);
	/* Clear overflow flag */
	TIFR0  = 1<<TOV0;
	TIFR1 = 1<<TOV0;
	/* Enable Overflow Interrupt */
	TIMSK1 = 1<< OCIE1A;

	/*setup timer 1 OCA for 1 sec given 1024 division */
	OCR1A = 15625;
	/*setup timer 1 OCB for 1 sec given 1024 division */
	OCR1B = 15625;
	
	/*reset TCNT of both timers */
	TCNT0 = 0;
	TCNT1 = 0;
}

//timer1 COMPA

ISR(TIMER1_COMPA_vect) {
	TCNT1 = 0;
	PORTB ^=(1<<led);
}

bool checkHanging(char inPin, char outPin, bool oldValue, int time)	/*Function to check if a pin is stalling*/
{
	/*	Pre-condition: Takes the inputPin and outputPin for each processor and compares them.	*/
	/*	Post-condition: Returns if the selected pins are stalling or not.						*/
	bool newValue = PORTD & (1 << inPin);
	if(newValue == oldValue)		/*If Pin Values are the same...*/
	{
		if(TCNT1 >= time)		/*Check to see if the timer has run out. If it has, then the pin is stalling and needs to be reset*/
		{		
			mustRebootPin = true;		/*Set the Flag to be true*/
			pinToReboot = outPin;		/*Store the pin number of the pin that is stalling*/
			//TCNT1 = 0;					/*Reset the timer*/
			time = TCNT1 + rebootTime;
			if(time >= 65000)
			{
				TCNT1 = 0;
				timer1 = TCNT1 + WAIT_TIME;
				timer2 = TCNT1 + WAIT_TIME;
				timer3 = TCNT1 + WAIT_TIME;
				time = TCNT1 + rebootTime;
			}
			/*
				timer45 = TCNT1 + 150;
				while(TCNT1 <= timer45)  
				{}		
				PORTC = 0x00;
				timer45 = TCNT1 + 150;			
				while(TCNT1 <= timer45)
				{}
				PORTC = 0xFF;	*/
		}
	}
	else						/*If the Values are different...*/
	{
		//TCNT1 = 0;					/*Reset the timer*/

		time = TCNT1 + rebootTime;
		if(time > 65000)
		{
			TCNT1 = 0;
			time = TCNT1 + rebootTime;
		}

		oldValue = newValue;			/*Set the read value to be the new value to compare against*/
	}
	if(inPin == sensor)
		timer1 = time;
	else if(inPin == router)
		timer2 = time;
	else if(inPin == led)
		timer3 = time;

	return oldValue;
}

void pinReboot(char pin)
{
	/*	Pre-Condition: Takes in the Pin determined to be stalling					*/
	/*	Post-Condition: Reboots pin and returns bootWaiting to be true				*/

		/* Set the Time constant we want to wait */		/* (IF THE PROGRAM IS TO BE UNIVERSAL FOR ALL THE CARDS, SET ALL TO THE COMPLETE_BOOT_TIME)*/
	if(pin == routerOut)		/*If Router was stalling, set the wait time to be equal to the time it takes for the router to reboot*/
	{
		rebootTime = COMPLETE_BOOT_TIME;
	}
	else if(pin == sensorOut)	/*If Sensor was stalling...*/
	{
		rebootTime = NAVI_BOOT_TIME;
	}
	else						/*If neither of the above, set to OTHER_BOOT_TIME*/
	{
		rebootTime = NAVI_BOOT_TIME;
	}
	
	pin = 0x00;				/*Set Pin Low*/
	/*Temp testing*/ PORTC = 0x00;

	TCNT0 = 0;						/*Timer Delay*/
	while(TCNT0 <= LOW_TIME) {}		/*Wait for the pin to register it is set LOW*/	

	pin = 0xFF;				/*Set Pin High*/
	/*Temp testing*/ PORTC = 0xFF;
	bootWaiting = true;				/*Trigger the bootWaiting flag for the main loop*/
	
		/*Set the new logic values*/
	oldValueSensor = sensor;
	oldValueLed = led;
	oldValueRouter = router;
	
	TCNT0 = 0;	
	TCNT1 = 0;					/*Reset the Timer*/
	
	return;
}