#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 8000000UL
#include "util/delay.h"


int swich=1;
int stepTemp=35,del=2;

ISR(INT0_vect){ // interrupt 0 sub routine
	if(swich == 0)
      swich = 1;
  	else
      swich = 0;
}

ISR(INT1_vect){ // interrupt 1 sub routine
	
  switch(swich){
    case 0:
  	stepTemp = (stepTemp+5);
	if(stepTemp==65){
		stepTemp=20;}
    case 1:
    del = (del+1)%5;
  }
}

void delay1s(){
  
  TCNT1H = 0x85;
  TCNT1L = 0xEE; // if the F is 16MHZ the register will be 0xBDC
  TCCR1B = 0x04;
  
  while((TIFR1&(1<<TOV1))==0);
 
  TCCR1B=0;
TIFR1=0x1;
}

void interruptConfig(){
	EIMSK = (1<<INT1)|(1<<INT0); // enable interrupt 1 and 0
	EICRA = 0x0F; // activate on rising edge
	PORTD |= (1<<2)|(1<<3);// activate pull up resistors
	sei();// enable interrupts
}

void adcConfig(){
	ADCSRA = 0x86; // Setting prescaler to CLK/64 and enabling ADC
	ADMUX = 0x00; // Adjusting results to right and selecting channel 0
}




int main(void)
{
	double analogV,temp,analogIn;
  	int tempO,tempT;
  	boolean lastState=false;
	DDRB = 0xFF; //set PORTB0 as output
  	DDRC &= 11111101; // PC1 as input
  	DDRD |= (1<<4);
  	int x=0;
  	
  	interruptConfig();
  	adcConfig();

  
    while(1)
    {
      switch(swich){
			case 0:
				ADCSRA = ADCSRA|(1<<6); // starting conversion
				while((ADCSRA&0b00010000)==0); // waiting until conversion is finished
				analogIn = (ADCH*256)+(ADCL); // getting the level number
				analogV = ((2*analogIn)/1024); // getting the voltage measurment
				temp = ((analogV*100)-50); // getting the temperaure 
        if (temp >= stepTemp){
					PORTB |= 0x01;  // if the temp higher or equal the step temp PORTB0 outputs 1
          	
        }
        else{
					PORTB &= 0xFE;  // if the temp lower than the step temp PORTB0 outputs 0
       }
        	
        		if (temp<0){ //if the tempreature is minus 
                  x=1; // turn the minus LED on
                  temp=temp*(-1);
                
              	}
        		else 
                  x=0;
        		tempT = (int )(temp/10); // finding the tens number
        		tempO = (int )(temp-(10*tempT)); // finding the ones number
        		
        		if (temp >99 ){ // if higher than 99 output 99
        		PORTC =0b00100100 ;
                PORTB =0b00010010;
                }
       			else{
        		PORTC = (tempT<<2); // output the tens
         		PORTB = (tempO<<1)| (x<<5);} // output the ones
        		break;
     		case 1:
     					
        	if((PINC & (1<<1))==0){ // check PINC1
              if(lastState==true){ // if 0 check if last time was 1
                for(int i=0; i<=del;i++){
                //delay1s();
                 _delay_ms(1000);
                }
                lastState=false;
               
                
                PORTD &= (0b11101111); // if last time was 1 delay then off the light
              }
              
              else
                PORTD &= (0b11101111);// if last time was 0 off without delay
        	}
        	else{
              PORTD |= (1<<4);// if PINC1 0 then turn on the light
              lastState = true;
            }
      			break;
      }
      			
    }
}