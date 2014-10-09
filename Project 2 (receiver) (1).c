#include <stdio.h>
#include <stdlib.h>
#include <at89lp828.h>
#include <at89lp6440.h>
// ~C51~
#define CLK 3686400L
#define BAUD 115200L
#define TIMER_2_RELOAD (0x10000L-(CLK/(16L*BAUD)))
#define hund 100
#define zero 0
//We want timer 0 to interrupt every 100 microseconds ((1/10000Hz)=100 us)
#define FREQ 10000L
#define TIMER0_RELOAD_VALUE (65536L-(CLK/FREQ))


//These variables are used in the ISR
volatile unsigned char pwmcount;
volatile unsigned char pwm1;
volatile unsigned char pwm2;
volatile unsigned char pwm3;
volatile unsigned char pwm4;
volatile unsigned char pwm5;
volatile unsigned char pencount;
volatile unsigned int distance;

void delay(void) 
{ 
int j, k; 
for(j=0; j<100; j++) 
{ 
for(k=0; k<1000; k++); 
} 
}
void SPIWrite(unsigned char value)
{
SPIF=0;
SPDR=value;
while(SPIF==0); //Wait for transmission to end
}
/*Read 10 bits from ithe MCP3004 ADC converter*/
unsigned int volatile GetADC(char channel)
{
unsigned int adc;
// initialize the SPI port to read the MCH3008 ADC attached to it.
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

int getVoltage(char channel)
{
	int voltage;
	voltage = GetADC(channel) * 8.0 / 1023.000;
	return voltage;
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
void Wait1S (void)
{
	_asm
	mov R2, #40
L3: mov R1, #250
L2: mov R0, #92
L1: djnz R0, L1 ; 2 machine cycles-> 2*0.27126us*184=100us
    djnz R1, L2 ; 100us*250=0.025s
    djnz R2, L3 ; 0.025s*40=1s
    _endasm;
} 

void Wait0_1S(void)
{
	_asm
	mov R2, #4
L4: mov R1, #250
L5: mov R0, #184
L6: djnz R0, L1 ; 2 machine cycles-> 2*0.27126us*184=100us
    djnz R1, L2 ; 100us*250=0.025s
    djnz R2, L3 ; 0.025s*40=1s
    _endasm;
} 
// Interrupt 1 is for timer 0.  This function is executed every time
// timer 0 overflows: 100 us.

void backward(void)
{
	
	pwm1 = 0;
	pwm2 = 30;
	pwm3 = 0;
	pwm4 = 30;
}

void forward(void)
{

	pwm1 = 30;
	pwm2 = 0;
	pwm3 = 30;
	pwm4 = 0;
}

void stop(void)
{
	pwm1 = 100;
	pwm2 = 100;
	pwm3 = 100;
	pwm4 = 100;
}

void turnLeft(void)
{
	pwm1 = 30;
	pwm2 = 0;
	pwm3 = 100;
	pwm4 = 100; 
}

void writeZero(void)
{
	P2_1 = 1;
	P2_3 = 0;
	P1_2 = 0;
	P1_1 = 0;
	P1_0 = 0;
	P3_7 = 0;
	P2_0 = 0;
}

void writeOne(void)
{
	P2_1 = 1;
	P2_3 = 1;
	P1_2 = 0;
	P1_1 = 0;
	P1_0 = 1;
	P3_7 = 1;
	P2_0 = 1;
}

void writeTwo(void)
{
	P2_1 = 0;
	P2_3 = 0;
	P1_2 = 0;
	P1_1 = 1;
	P1_0 = 0;
	P3_7 = 0;
	P2_0 = 1;
}

void writeThree(void)
{
	P2_1 = 0;
	P2_3 = 0;
	P1_2 = 0;
	P1_1 = 0;
	P1_0 = 0;
	P3_7 = 1;
	P2_0 = 1;
}

void writeFour(void)
{	
	P2_1 = 0;
	P2_3 = 1;
	P1_2 = 0;
	P1_1 = 0;
	P1_0 = 1;
	P3_7 = 1;
	P2_0 = 0;
}

void writeFive(void)
{
	P2_1 = 0;
	P2_3 = 0;
	P1_2 = 1;
	P1_1 = 0;
	P1_0 = 0;
	P3_7 = 1;
	P2_0 = 0;
}

void turnRight(void)
{
	pwm1 = 100;
	pwm2 = 100;
	pwm3 = 30;
	pwm4 = 0;                                             
}

void manualForward(void)
{
	if(distance>0)
	distance--;
}

void manualBackward(void)
{
	if(distance<5)
	distance++;
}
void checkdistance(void)
{
	if(distance==0)
	writeZero();
	else if(distance ==1)
	writeOne();
	else if(distance ==2)
	writeTwo();
	else if(distance ==3)
	writeThree();
	else if(distance ==4)
	writeFour();
	else if(distance ==5)
	writeFive();
}

void forwardE(void)
{
	pwm1 = 100;
	pwm2 = 0;
	pwm3 = 100;
	pwm4 = 0;
}
void backwardE(void)
{
	pwm1 = 0;
	pwm2 = 100;
	pwm3 = 0;
	pwm4 = 100;
}
void turnleftE(void)
{
	pwm1 = 100;
	pwm2 = 0;
	pwm3 = 100;
	pwm4 = 100;
}
	
	
void motion(void)
{
	int volt1 = getVoltage(2)+1;
	int volt2 = getVoltage(3); 
	if(volt1 == 0 && volt2 == 0){
		stop();
		printf("stop");
		checkdistance();
	}
	else if(distance == 5){
		backward();
		printf("moving backward");
		checkdistance();}
	else
	{
		if(volt1 == volt2)
		{
	
			//if(volt1 == 0 || volt2 == 0)
			//{
			//	stop();
			//	printf("stop");
			//	checkdistance();
			//}

			if(volt1 == distance || volt2 == distance)
			{
				stop();
				printf("stop");
				checkdistance();
			} 
					
			else if(volt1 < distance || volt2 < distance)
			{
				backward();
				printf("moving backward");
				checkdistance();
			}
			else if(volt1 > distance || volt2 > distance)
			{
				forward();
				printf("moving forward");
				checkdistance();
			}
		}

		else if(volt1 < volt2 )
		{
			turnRight();
			printf("turning right");
			checkdistance();
		}
	
		else if(volt1 > volt2)
		{
			turnLeft();
			printf("turning left");
			checkdistance();
		}
	}

}

void penDown(void){
pwm5 = 5;
}

void penUp(void){
pwm5 = 14;
}

void drawE(void)
{
int i;
penDown();
Wait1S();
forwardE();
for(i =0; i<4 ; i++){
Wait0_1S();
}
stop();

penUp();
Wait1S();
backwardE();
for(i =0; i<7 ; i++){
Wait0_1S();
}
turnleftE();
for(i =0; i<9 ; i++){
Wait0_1S();
}
forwardE();
for(i =0; i<6 ; i++){
Wait0_1S();
}
stop();

penDown();
Wait1S();
forwardE();
for(i =0; i<8 ; i++){
Wait0_1S();
}
stop();

penUp();
Wait1S();
backwardE();
for(i =0; i<8 ; i++){
Wait0_1S();
}
turnleftE();
for(i =0; i<14 ; i++){
Wait0_1S();
}
forwardE();
for(i =0; i<5 ; i++){
Wait0_1S();
}
stop();
penDown();
Wait1S();
forwardE();
for(i =0; i<9 ; i++){
Wait0_1S();
}
stop();
penUp();
Wait1S();
backwardE();
for(i =0; i<8 ; i++){
Wait0_1S();
}
turnleftE();
for(i =0; i<14 ; i++){
Wait0_1S();
}
forwardE();
for(i =0; i<8 ; i++){
Wait0_1S();
}
stop();
backwardE();
for(i =0; i<7 ; i++){
Wait0_1S();
}
turnleftE();
for(i =0; i<9 ; i++){
Wait0_1S();
}
forwardE();
for(i =0; i<6 ; i++){
Wait0_1S();
}
stop();
penDown();
Wait1S();
forwardE();
for(i =0; i<6 ; i++){
Wait0_1S();
}
stop();
penUp();
}

void checkManual(void)
{
	int time=0;
	while(getVoltage(2) == 0)
	{
		Wait0_1S();
		time++;
	}
	
	if(time >= 4 && time <= 12)
	manualForward();
	if(time >= 14 && time <= 23)
	manualBackward();
	if(time >= 26 && time <= 33)
	 drawE();
		
	printf("time = %d\n",time);
    motion();//forwardE();
    time=0;
}




void pwmcounter (void) interrupt 1
{
	if(++pwmcount>99) pwmcount=0;
	P3_2=(pwm1>pwmcount)?1:0;
	P3_3=(pwm2>pwmcount)?1:0;
	P3_4=(pwm3>pwmcount)?1:0;
	P3_5=(pwm4>pwmcount)?1:0;
	
	if(++pencount>199) pencount=0;
	P1_3=(pwm5>pencount)?1:0;
}


void main (void)
{
	CLKREG=0x00; // TPS=0000B
	setbaud_timer2(TIMER_2_RELOAD); // Initialize serial port using timer 2 
	InitTimer0(); // Initialize timer 0 and its interrupt
	pwm1 = 100;
	pwm2 = 100;
	pwm3 = 100;
	pwm4 = 100;
	
	distance=3; 
	penUp();
	//writeTwo();
	//forwardE();
	//Wait0_1S();
	//stop();
	while(1)
	{
	



		printf("channel 0 = %d\n",getVoltage(2)+1);
		printf("channel 1 = %d\n",getVoltage(3));
		printf("Distance = %d\n",distance);
		Wait1S();
		checkManual();
		printf("\n");
		
		
	}
	
}



