/*
* AutoMove.c
*
* Created: 06/02/2022 9:33:32 AM
* Author : HP
*/

#define F_CPU 8000000UL
#include <avr/interrupt.h> //ISR() functions are defined in here
#include <util/delay.h>
#include <stdlib.h>
#include <avr/io.h>

static volatile unsigned int tCount = 0;//to hold the counter value from TCNT1.
static volatile int i = 0; //to indicate the state of ultrasonic sensor.
double distance;//to store the measured distance in cm

void trig(); //to send pulse

void goAhead();
void goLeft();
void stopRobot();
void isBlocked();

int main(void)
{
	
	DDRC = DDRC |(1 << DDC3); // Set pin as an output
	DDRC = DDRC |(1 << DDC4);
	DDRC = DDRC |(1 << DDC5);
	DDRC = DDRC |(1 << DDC6);
	DDRA = DDRA |(1 << DDA1); // to set Trigger pin as an output
	DDRD = DDRD & ~(1 << DDD2) ; // to set Echo pin(INT0 - PD2 on atmega32) as an input.
	PORTD = PORTD & ~(1 << PD2) ; 
	_delay_ms(50);
	
	GICR |= (1<<INT0); //to enable interrupt for INT0
	MCUCR |= (1<<ISC00);//to make any logical change on INT0 generate an interrupt
	TIMSK |= (1 << TOIE1);//to enable Timer/Counter 1, Overflow interrupt
	
	sei();//to enable global interrupt bit
	_delay_ms(500);
	
	while(1)
	{
		
		isBlocked(); //to call isBlocked function
		_delay_ms(200);
		
	}
}

void isBlocked(){
	//to control the motors
	trig();
	if (distance < 5)
	{
		stopRobot();
		_delay_ms(2000);
		goLeft();
		_delay_ms(2000);
		stopRobot();
		isBlocked(); // recursively call function 
	}else{
		goAhead();
	}
}


ISR(INT0_vect){
	// This function will get executed whenever the micro controller is interrupted by INT0.
	if(i== 0)//to get executed when interrupted by a rising edge(for the first time, since i = 0)
	{
		TCCR1B |=(1<<CS10);//to start counting(No p rescaling). See page number 110
		i=1;
	}
	else//to get executed when interrupted by the falling edge(for the second time, since we set i = 1 previously)
	{
		TCCR1B = 0;//to stop counting
		tCount = TCNT1;//to take the value of counter to our variable
		distance = tCount *1000000.0/F_CPU/58;
		TCNT1 = 0;//to reset counter to 0
		i=0; 
	}
}

ISR(TIMER1_OVF_vect)
{// This function will get executed if Timer/counter 1 get overflowed
	TCCR1B = 0;//to stop counting
	tCount = 65535;//to make the variable ~0 when there is a counter overflow
	distance = tCount *1000000.0/F_CPU/58;
	
	TCNT1 = 0;//to reset counter to 0
	i = 0;
}

void trig()
{
	//Send a 15us pulse to the trigger pin
	PORTA = PORTA |(1<<PA1);
	_delay_us(15);
	PORTA = PORTA &= ~(1<<PA1);
	_delay_us(15);
}



void goAhead(){

	//Left Motor
	PORTC = PORTC | (1 << DDC3);
	PORTC = PORTC & ~(1 << DDC4);
	
	//Right Motor
	PORTC = PORTC | (1 << DDC5);
	PORTC = PORTC & ~(1 << DDC6);
}


void goLeft(){
	//Left Motor
	PORTC = PORTC & ~(1 << DDC3); //stop
	PORTC = PORTC & ~(1 << DDC4);

	//Right Motor
	PORTC = PORTC | (1 << DDC5); //rotate
	PORTC = PORTC & ~(1 << DDC6);
}


void stopRobot(){
	//Left Motor
	PORTC = PORTC & ~(1 << DDC3); // stop
	PORTC = PORTC & ~(1 << DDC4);
	//Right Motor
	PORTC = PORTC & ~(1 << DDC5); // stop
	PORTC = PORTC & ~(1 << DDC6);
}
