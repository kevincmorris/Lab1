// ******************************************************************************************* //
// Include file for PIC24FJ64GA002 microcontroller. This include file defines
// MACROS for special function registers (SFR) and control bits within those
// registers.

// Nicolas Fajardo, Kevin Morris, & Paul Cross
// Team 202
// Date modified: 10/06/2014

// K.N.P. -- Know Nonsense Programmers

#include "p24fj64ga002.h"
#include <stdio.h>
#include <stdlib.h>
#include "lcd.h"

volatile int state = 0;
volatile unsigned int minutes = 0;
volatile unsigned int seconds = 0;
volatile char buf[3];

void DebounceDelay(void);

// ******************************************************************************************* //
// Configuration bits for CONFIG1 settings.
//
// Make sure "Configuration Bits set in code." option is checked in MPLAB.
// This option can be set by selecting "Configuration Bits..." under the Configure
// menu in MPLAB.
//
// These settings are appropriate for debugging the PIC microcontroller. If you need to
// program the PIC for standalone operation, change the COE_ON option to COE_OFF.

_CONFIG1( JTAGEN_OFF & GCP_OFF & GWRP_OFF &
          BKBUG_ON & COE_ON & ICS_PGx1 &
          FWDTEN_OFF & WINDIS_OFF & FWPSA_PR128 & WDTPS_PS32768 )

// ******************************************************************************************* //
// Configuration bits for CONFIG2 settings.
// Make sure "Configuration Bits set in code." option is checked in MPLAB.
// This option can be set by selecting "Configuration Bits..." under the Configure
// menu in MPLAB.

_CONFIG2( IESO_OFF & SOSCSEL_SOSC & WUTSEL_LEG & FNOSC_PRIPLL & FCKSM_CSDCMD & OSCIOFNC_OFF &
          IOL1WAY_OFF & I2C1SEL_PRI & POSCMOD_XT )


// ******************************************************************************************* //
// Defines to simply UART's baud rate generator (BRG) regiser
// given the osicllator freqeuncy and PLLMODE.

#define XTFREQ          7372800         	  // On-board Crystal frequency
#define PLLMODE         4               	  // On-chip PLL setting (Fosc)
#define FCY             (XTFREQ*PLLMODE)/2    // Instruction Cycle Frequency (Fosc/2)

#define BAUDRATE         115200
#define BRGVAL          ((FCY/BAUDRATE)/16)-1

// ******************************************************************************************* //

int main(void)
{
	// ****************************************************************************** //

	LCDInitialize();

	// Configure AD1PCFG register for configuring input pins between analog input
	// and digital IO.
	AD1PCFGbits.PCFG4 = 1;

	// Configure TRIS register bits for Right and Left LED outputs.
	TRISAbits.TRISA0 = 0;	// Run LED
	TRISAbits.TRISA1 = 0;	// Stop LED

	// Configure LAT register bits to initialize Right LED to on.
	LATAbits.LATA0 = 1; // LEFT LED ON
	LATAbits.LATA1 = 0; // RIGHT LED OFF

	// Configure ODC register bits to use open drain configuration for Right
	// and Left LED output.
	ODCAbits.ODA0 = 1;
	ODCAbits.ODA1 = 1;	

	// Configure TRIS register bits for switch input.
	TRISBbits.TRISB2 = 1;
	TRISBbits.TRISB5 = 1;

	// Configure CNPU register bits to enable internal pullup resistor for switch input.
	CNPU1bits.CN6PUE = 1;
	
	// This line sets the Timer 2 to use the internal clock, to have a prescaler of 256,
	// and to start in the 'off' position.
	T3CON = 0x0030;
	T1CON = 0x0030;

	// Clear Timers' values and reset interrupt flags
	TMR3 = 0;
	IFS0bits.T3IF = 0;
	IEC0bits.T3IE = 1;
	
	TMR1 = 0;
	IFS0bits.T1IF = 0;
	IEC0bits.T1IE = 1;

	// Set periods of 
	PR3 = 287;
	PR1 = 57599;

	// set change interrupt for both switches
	CNEN2bits.CN27IE = 1;
	CNEN1bits.CN6IE = 1;
	IFS1bits.CNIF = 0;
	IEC1bits.CNIE = 1;
	
	// Ensure LCD gets cleared before starting functionality
	LCDClear();
	while(1) {
		

	/* 	This block of code has the three states of the stopwatch.  Case 0 is the initial state
		with the red LED on and the timer stopped with "Stop" printed on the LCD.  Case 1 is the
		running state with the screen incrementing by the timer and "Run" printed on the LCD.
		Case 2 is the stopped state after the stopwatch has been used.  The timer value is still
		printed on the LCD and "Stop" is printed.
	*/
		switch (state) {
			case 0:
					LATAbits.LATA0 = 1;		// red on
					LATAbits.LATA1 = 0;		
					LCDMoveCursor(0,0);
					LCDPrintString("Stop:");
					LCDMoveCursor(1,0);
					LCDPrintString("00:00.00");
				break;
			case 1:
					LATAbits.LATA0 = 0;
					LATAbits.LATA1 = 1;		// green on
					LCDMoveCursor(0,0);
					LCDClear();
					LCDPrintString("Run:");
					LCDMoveCursor(1,0);
					
		// these conditional statements separate the numbers and print them properly
					if (minutes < 10){
						LCDPrintChar('0'); 
						LCDPrintChar(minutes + '0');	
					}
					else if (minutes >= 10) {
						LCDPrintChar( minutes/10 + '0');
						LCDPrintChar( minutes%10 + '0');
					}
					LCDPrintChar(':');

					if (seconds < 10){	
						LCDPrintChar('0'); 
						LCDPrintChar(seconds + '0');
					}
					else if ( seconds >= 10) {
						LCDPrintChar( seconds/10 + '0');
						LCDPrintChar( seconds%10 + '0');
					}
					LCDPrintChar('.');

					if (TMR1/576 < 10){	
						LCDPrintChar('0'); 
						LCDPrintChar(TMR1/576 + '0');
					}
					else if ( TMR1/576 >= 10) {
						LCDPrintChar( TMR1/5760 + '0');
						LCDPrintChar( (TMR1/576)%10 + '0');
					}
				break;
			case 2:
					LATAbits.LATA0 = 1;		// red on
					LATAbits.LATA1 = 0;
					LCDMoveCursor(0,0);
					LCDClear();
					LCDPrintString("Stop:");
					LCDMoveCursor(1,0);

			// these conditional statements separate the numbers and print them properly
					if (minutes < 10){
						LCDPrintChar('0'); 
						LCDPrintChar(minutes + '0');	
					}
					else if (minutes >= 10) {
						LCDPrintChar( minutes/10 + '0');
						LCDPrintChar( minutes%10 + '0');
					}
					LCDPrintChar(':');

					if (seconds < 10){	
						LCDPrintChar('0'); 
						LCDPrintChar(seconds + '0');
					}
					else if ( seconds >= 10) {
						LCDPrintChar( seconds/10 + '0');
						LCDPrintChar( seconds%10 + '0');
					}
					LCDPrintChar('.');

					if (TMR1/576 < 10){	
						LCDPrintChar('0'); 
						LCDPrintChar(TMR1/576 + '0');
					}
					else if ( TMR1/576 >= 10) {
						LCDPrintChar( TMR1/5760 + '0');
						LCDPrintChar( (TMR1/576)%10 + '0');
					}
				break;
			
		}
		

		// TODO: Use DebounceDelay() function to debounce button press
		// and button release in software.
	}
	return 0;
}
// *******************************************************************************************

void DebounceDelay(void) {	// function declaration for debouncing
	T3CONbits.TON = 1;  // turn on TMR3 to activate 5ms interrupt cycle
	IFS0bits.T3IF = 0;
	while(IFS0bits.T3IF == 0){};	// waits for the full timer to execute
	T3CONbits.TON = 0;
	IFS0bits.T3IF = 0;
}

void __attribute__((interrupt,auto_psv)) _T1Interrupt(void){
	++seconds;
	if (seconds == 60) {	// increment minutes if the seconds has reached 60
		seconds = 0;
		++minutes;
	}
    TMR1 = 0;				// reset TMR1 value
    IFS0bits.T1IF = 0;		// drop interrupt flag to be ready for next interrupt
}


void __attribute__((interrupt,auto_psv)) _CNInterrupt(void){
	
	IFS1bits.CNIF = 0;
	DebounceDelay();

	switch (state){
		case 0:								// Initial state
			if(PORTBbits.RB5 == 0){			// if reset
				state = 0;
			}
			else if(PORTBbits.RB2 == 0){	// if external switch pressed
				state = 1;
				_TON = 1;
			}
			break;
		case 1:								// After switch is pressed, before second press
			if(PORTBbits.RB2 == 0){			// if external switch is pressed, stop timer and proceedS
				state = 2;
				_TON = 0;
			}
			
			break;
		case 2:								// After switch is pressed second time, before release
			if(PORTBbits.RB2 == 0){
				state = 1;
				_TON = 1;
			}
			if (PORTBbits.RB5 == 0){		// if reset is pressed return to init 
				state = 0;
				minutes = 0;
				seconds = 0;
				_TON = 0;
				TMR1 = 0;
			}
			
			break;
	
	}

}