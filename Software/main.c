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

#define INVERT_LEDS					(GPIO_LED ^= 0xff)

#define ARRAY_SIZE(__x__)   (sizeof(__x__)/sizeof(__x__[0]))

volatile uint8  counter  = 0; // current number of char received on UART currently in RxBuf[]
volatile uint8  BufReady = 0; // Flag to indicate if there is a sentence worth of data in RxBuf
volatile uint8  RxBuf[BUF_SIZE];


//////////////////////////////////////////////////////////////////
// Interrupt service routine, runs when UART interrupt occurs - see cm0dsasm.s
//////////////////////////////////////////////////////////////////
void UART_ISR()		
{
	char c;
	c = UART_RXD;	 // read a character from UART - interrupt only occurs when character waiting
	RxBuf[counter]  = c;   // Store in buffer
	counter++;             // Increment counter to indicate that there is now 1 more character in buffer
	UART_TXD = c;   // write (echo) the character to UART (assuming transmit queue not full!)
	// counter is now the position that the next character should go into
	// If this is the end of the buffer, i.e. if counter==BUF_SIZE-1, then null terminate
	// and indicate the a complete sentence has been received.
	// If the character just put in was a carriage return, do likewise.
	if (counter == BUF_SIZE-1 || c == ASCII_CR)  
	{
		counter--;							// decrement counter (CR will be over-written)
		RxBuf[counter] = NULL;  // Null terminate
		BufReady       = 1;	    // Indicate to rest of code that a full "sentence" has being received (and is in RxBuf)
	}
}

//////////////////////////////////////////////////////////////////
// Interrupt service routine for System Tick interrupt
//////////////////////////////////////////////////////////////////
void SysTick_ISR()	
{
	// Do nothing - this interrupt is not used here
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
	uint8 TxBuf[ARRAY_SIZE(RxBuf)];

	// Configure the UART - the control register decides which events cause interrupts
	UART_CTL = (1 << UART_RX_FIFO_NOTEMPTY_BIT_POS);	// Enable rx data available interrupt only
	
	// Configure the interrupt system in the processor (NVIC)
	NVIC_Enable = (1 << NVIC_UART_BIT_POS);		// Enable the UART interrupt
	
	delay(nLOOPS_per_DELAY);						// wait a little
	
	printf("\nWelcome to Cortex-M0 SoC\n");		// output welcome message
	
	while(1)		// loop forever
	{	
		// Do some processing before entering Sleep Mode
		GPIO_LED = GPIO_SW;	// echo the switches onto the LEDs
		delay(nLOOPS_per_DELAY);		// delay a little
		INVERT_LEDS;						// invert the 8 rightmost LEDs
		delay(nLOOPS_per_DELAY);
		INVERT_LEDS;						// invert the same LEDs again
		delay(nLOOPS_per_DELAY);
		
		printf("\nType some characters: ");  // invite some input
		
		while (BufReady == 0)	// loop until enough characters have been collected
		{
			__wfi();  // Wait For Interrupt: enter Sleep Mode - wake on character received
			// only get to this point if a character has been received
			GPIO_LED_Lo = RxBuf[counter-1];  // display the code for the character
		}

		// get here when CR entered or buffer full - do some processing with interrupts disabled
		NVIC_Disable = (1 << NVIC_UART_BIT_POS);	// Disable the UART interrupt

		// ---- start of critical section ----
		for (i=0; i<=counter; i++)		// step through the received character array
		{
			if (RxBuf[i] >= 'A') 				// if this character is a letter (roughly)
				TxBuf[i] = RxBuf[i] ^ CASE_BIT;  // copy to transmit buffer, changing case
			else 
				TxBuf[i] = RxBuf[i];             // otherwise copy with no change
		}
		
		BufReady = 0;	// clear the flag
		counter  = 0; // clear the counter for the next time	
		// ---- end of critical section ----		
		
		NVIC_Enable = (1 << NVIC_UART_BIT_POS);		// Enable the UART interrupt

		printf("\n:--> |%s|\n", TxBuf);  // print the results between bars

	} // end of infinite loop

}  // end of main


