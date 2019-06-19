/*
	Timers.h
	Timer Include File
	By: Scott Grisso
*/
#include "Timers.h"

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
	//TIMSK0 = 1<< OCIE0A;

	OCR0A = 255;
	//OCR0B = 255;

	/*setup timer 1 OCA for 1 Msec given 1024 division */
	OCR1A = 16;
	/*setup timer 1 OCB for 1 Msec given 1024 division */
	//OCR1B = 15625;
	
	/*reset TCNT of both timers */
	TCNT0 = 0;
	TCNT1 = 0;
}
//TIMER1 COMPA
ISR(TIMER1_COMPA_vect) {
	TCNT1 = 0;
	globalTimerTracker( );
	//PORTB = PORTB ^(1 << PORTB5);
	//TIFR1 = 1<<TOV1;
}

ISR(TIMER0_COMPA_vect)
{
	TCNT0 = 0;
	TIFR0  = 1<<TOV0;
	globalTimerTracker();
}

//*****************************************
//********TIMER.C file code****************
//*****************************************
#include "Timers.h"
#include "stdlib.h"
unsigned long globalTime;

unsigned long millis(void)
{
    return globalTime;
}

bool timerDone(timer_t * t)
{
    if(abs(millis()-t->lastMillis) > t->timerInterval)
    {
        t->lastMillis=millis();
        return true;
    }
    else
    {
        return false;
    }
}

bool timerDoneNoReset(timer_t * t)
{
	if(abs(millis()-t->lastMillis) > t->timerInterval)
	{
		return true;
	}
	else
	{
		return false;
	}
	
}

void setTimerInterval(timer_t * t, unsigned long interval)
{
    t->timerInterval= interval;
}

void resetTimer(timer_t * t)
{
    t->lastMillis=millis();
}

//Call this function in your timer interrupt that fires at 1ms
void globalTimerTracker( )
{
    globalTime++;
}

//*****************************************
//**********    END OF C file  ************
//*****************************************