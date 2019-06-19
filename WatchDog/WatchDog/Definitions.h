/*
 * Definitions.h
 *
 * Created: 4/28/2018 5:21:22 PM
 *  Author: John
 */ 
#ifndef DEFINITIONS_H
#define DEFINITIONS_H


/*Enable Cards: Comment out if you don't want to use*/
//#define CARD1 PORTD2
#define CARD2  PORTD3
//#define CARD3 PORTD4
/*	PORTD for INPUT		*/

/*	PORTC for OUTPUT	*/
#define CARD1Out PORTC3 
#define CARD2Out  PORTC0 
#define CARD3Out PORTC1

/*New time constants*/
#define COMPLETE_BOOT_TIME	7000
#define TIME_INACTIVE		2000
#define NAVI_BOOT_TIME		650
#define OTHER_BOOT_TIME		400
#define HIGH_TIME	250
#define WAIT_TIME	450

#endif