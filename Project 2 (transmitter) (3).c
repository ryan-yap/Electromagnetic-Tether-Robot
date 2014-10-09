#include <stdio.h>
#include <stdlib.h>
#include <at89lp828.h>

#define CLK 3686400L
#define BAUD 115200L
#define TIMER_2_RELOAD (0x10000L-(CLK/(16L*BAUD)))

//We want timer 0 to interrupt every 100 microseconds ((1/10000Hz)=100 us)
#define FREQ 30000L
#define TIMER0_RELOAD_VALUE (65536L-(CLK/FREQ))

//These variables are used in the ISR
volatile unsigned char pwmcount;
volatile unsigned char pwm1;
volatile unsigned int one;
void Wait0_1S(void)
{
	_asm
	mov R2, #4
L3: mov R1, #250
L2: mov R0, #184
L1: djnz R0, L1 ; 2 machine cycles-> 2*0.27126us*184=100us
    djnz R1, L2 ; 100us*250=0.025s
    djnz R2, L3 ; 0.025s*40=1s
    _endasm;
} 


void InitTimer0 (void)
{
	// Initialize timer 0 for ISR 'pwmcounter' below
	TR0=0; // Stop timer 0
	TMOD=(TMOD&0xf0)|0x01; // 16-bit timer
	RH0=TIMER0_RELOAD_VALUE/0x100;
	RL0=TIMER0_RELOAD_VALUE%0x100;
	TR0=1; // Start timer 0 (bit 4 in TCON)
	ET0=1; // Enable timer 0 interrupt
	EA=1;  // Enable global interrupts
}

void SPIWrite(unsigned char value)
{
	SPIF=0;
	SPDR=value;
	while(SPIF==0); //Wait for transmission to end
}

int GetADC(char channel)
{
	unsigned int adc;	// initialize the SPI port to read the MCH3008 ADC attached to it.
	SSIG=1;
	SPCR=SPE|MSTR|CPOL|CPHA|SPR1|SPR0; //Mode (1,1): see figure 6-1 of MCP3004 datasheet.
	
	P1_4=0; //Select the MCP3004 converter.
	
	SPIWrite(0x01);//Send the start bit.
	SPIWrite((channel*0x10)|0x80);	//Send single/diff* bit, D2, D1, and D0 bits.
	adc=((SPDR & 0x03)*0x100);//SPDR contains now the high part of the result.
	SPIWrite(0x55);//Dont' care what you send now. 0x55 looks good on the oscilloscope though!
	adc+=SPDR;//SPDR contains now the lows part of the result. 
	
	P1_4=1; //Deselect the MCP3004 converter.
	
	return adc;
}
/*void InitTimer1 (void)
{
	// Initialize timer 0 for ISR 'pwmcounter' below
	TR1=0; // Stop timer 0
	TMOD=(TMOD&0xf0)|0x01; // 16-bit timer
	RH1=TIMER1_RELOAD_VALUE/0x100;
	RL1=TIMER1_RELOAD_VALUE%0x100;
	TR1=1; // Start timer 0 (bit 4 in TCON)
	ET1=1; // Enable timer 0 interrupt
	EA=1;  // Enable global interrupts
}*/

void forward_signal(void)
{
	int i;
	one=0;
	for (i=0; i<3 ; i++){
	Wait0_1S();
	}
	one=1;
}	

void backward_signal(void)
{	
	int i;
	one=0;
	for (i=0; i<6 ; i++){
	Wait0_1S();
	}
	one=1;
}	

void E_signal(void)
{	
	int i;
	one=0;
	for (i=0; i<9 ; i++){
	Wait0_1S();
	}
	one=1;
}

void checkbutton(void)
{
    //here transmitter is still sending a sinasoudal wave
	while(GetADC(0)>680)
	{ 
		while(GetADC(0)>680);//wait for the user to release the button
	 	forward_signal();
	}
	 
	while(GetADC(1)>680)
	{
		while(GetADC(1)>680);//wait for the user to release the button
		backward_signal(); 
	} 
	
	while(GetADC(2)>680)
	{
		while(GetADC(2)>680);//wait for the user to release the button
		E_signal(); 
	} 
	

}


// Interrupt 1 is for timer 0.  This function is executed every time
// timer 0 overflows: 100 us.
void pwmcounter (void) interrupt 1
{
	
	
		if(++pwmcount>1) pwmcount=0;
		P3_4=(pwm1>pwmcount)?one:0;
		P3_5=(pwm1>pwmcount)?0:one;
	//	printf("P2_6=%d\n",P2_6);
	
	/*
	else if(P2_6==1)
	{
		if(++pwmcount>1) pwmcount=0;
		P3_4=(pwm1>pwmcount)?0:0;
		P3_5=(pwm1>pwmcount)?0:0;
	//	printf("P2_6=%d\n",P2_6);
	}
	*/
//	
}

void main (void)
{
	volatile int i=1;
//	int j;
	//int counter;
    setbaud_timer2(TIMER_2_RELOAD); // Initialize serial port using timer 2 
	InitTimer0(); // Initialize timer 0 and its interrupt
	pwm1=1;
	one=1;
	while(i==1)
	{
	//this tells you there is a sinasoudal wave
		//if(P2_6==1)
		 
		checkbutton();

		
		
	}	

}

