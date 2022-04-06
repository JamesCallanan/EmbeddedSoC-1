//------------------------------------------------------------------------------------------------------
// Demonstration program for Cortex-M0 SoC design - simple version, no CMSIS
// 1)Enable the interrupt for UART - interrupt when character received
// 2)Go to sleep mode
// 3)On interruption, echo character back to the UART and collect into buffer
// 4)When a whole sentence has been received, invert the case and send it back
// 5)Loop forever doing the above.
//
// Version 5 - March 2022
//------------------------------------------------------------------------------------------------------
#include <stdio.h>
#include "DES_M0_SoC.h"

#define BUF_SIZE						100
#define ASCII_CR						'\r'
#define CASE_BIT						('A' ^ 'a')
#define nLOOPS_per_DELAY		1000000

#define INVERT_LEDS					(gpioLed ^= 0xff)

#define ARRAY_SIZE(__x__)   (sizeof(__x__)/sizeof(__x__[0]))

#define NUM_DIGITS 4 	// Number of digits enabled in 7 segment display

volatile uint8  counter  = 0; // current number of char received on UART currently in RxBuf[]
volatile uint8  BufReady = 0; // Flag to indicate if there is a sentence worth of data in RxBuf
volatile uint8  RxBuf[BUF_SIZE];

//////////////////////////////////////////////////////////////////
// Interrupt service routine for System Tick interrupt
//////////////////////////////////////////////////////////////////
void SysTick_ISR()	
{
	// Do nothing - this interrupt is not used here
}

//////////////////////////////////////////////////////////////////
// Interrupt service routine, runs when UART interrupt occurs - see cm0dsasm.s
//////////////////////////////////////////////////////////////////
void UART_ISR()		
{
	
}

// function to display temperature code
uint32 dispTempLED(uint8 number)
{
	uint32 gpioLed;
	
	switch(number)
	{
		case 0:
			gpioLed = 0x0000;
			break;
			
		case 1:
			gpioLed = 0x0001;
			break;
		
		case 2:
			gpioLed = 0x0003;
			break;
			
		case 3:
			gpioLed = 0x0007;
			break;
			
		case 4:
			gpioLed = 0x000F;
			break;
			
		case 5:
			gpioLed = 0x001F;
			break;
			
		case 6:
			gpioLed = 0x003F;
			break;
			
		case 7:
			gpioLed = 0x007F;
			break;
			
		case 8: 
			gpioLed = 0x00FF;
			break;
			
		case 9:
			gpioLed = 0x01FF;
			break;
			
		case 10:
			gpioLed = 0x03FF;
			break;
			
		case 11:
			gpioLed = 0x07FF;
			break;
			
		case 12:
			gpioLed = 0x0FFF;
			break;
			
		case 13:
			gpioLed = 0x1FFF;
			break;
			
		case 14:
			gpioLed = 0x3FFF;
			break;
			
		case 15:
			gpioLed = 0x7FFF;
			break;
			
		case 16:
			gpioLed = 0xFFFF;
			break;
			
		default:
			gpioLed = 0x0000;
			break;
	}
	return gpioLed;
}

// configuration function for 7 segment display
void segDispConfig()
{
	// number of digits to be enabled on display (enable all digits)
	ENBL_DIG = 0xFF;
	
	// we want all 8 digits to be in raw mode (all 0)
	SET_MODE = 0x00;
	
	// we want all dots off (for now)
	CTRL_DOT = 0x00;
}
	
// function to send raw data to raw registers in 7-segment display
void sendRaw(uint32 raw, int low)
{
	if(low)
	{
		RAW_LOW = raw;
	}
	else
	{
		RAW_HIGH = raw;
	}	
}

// function which maps from integer to 8 bit hex for 7 segment display
uint8 map2segDisp(uint8 digit)
{
	uint8 raw;
	switch(digit)
	{
		case 0:
			raw = 0xFA;
			break;
			
		case 1:
			raw = 0x60;
			break;
			
		case 2:
			raw = 0xDA;
			break;
			
		case 3:
			raw = 0xF2;
			break;
			
		case 4:
			raw = 0x66;
			break;
			
		case 5:
			raw = 0xB6;
			break;
			
		case 6:
			raw = 0xBE;
			break;
			
		case 7:
			raw = 0xE0;
			break;
			
		case 8:
			raw = 0xFE;
			break;
			
		case 9:
			raw = 0xF6;
			
		case 10:
			raw = 0x02; // Case for minus sign
			
		default:
			raw = 0x00;
	}
	return raw;
}
	
// function to display signed number on 7 segment 
void displayNumber(int number)
{
	int sign = 0;
	int i;
	uint8 rawDigit;
	uint8 digitValue;
	
	if(number < 0)
	{
		sign = 1;
		number = -number;
	}
	
	for(i=1; i<=NUM_DIGITS; i++)
	{
		digitValue = number % 10;
		rawDigit = map2segDisp(digitValue);
		sendRaw(rawDigit,1);
		number = number/10;
		
		if(i==NUM_DIGITS && sign==1)
		{
			rawDigit = map2segDisp(10);
			sendRaw(rawDigit,1);
		}
	}
	
}

	
//////////////////////////////////////////////////////////////////
// Software delay function
// As a rough guide, looping 1000 times takes 160 us to 220 us, 
// depending on the compiler optimisation level.
//////////////////////////////////////////////////////////////////
void delay(uint32 n) 
{
	volatile uint32 i;
	for(i=0;i<n;i++)
	{
	}
}


//////////////////////////////////////////////////////////////////
// Main Function
//////////////////////////////////////////////////////////////////
int main(void) 
{
	uint8 i;
	int j;
	
	// configure 7 segment display
	segDispConfig();
	
	while(1)		// loop forever
	{	
		/*// loop through the temperature LED code indefinitely
		for(i=0;i<17;i++)
		{
			// display temperature code for i 
			GPIO_LED = dispTempLED(i);
			displayNumber(i);
			delay(4000000);
		}*/
		
		// loop through the temperature LED code indefinitely
		for(j=-10;j<=10;j++)
		{
			displayNumber(j);
			delay(4000000);
		}

	} // end of infinite loop

}  // end of main
