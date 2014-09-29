// ******************************************************************************************* //
// Include file for PIC24FJ64GA002 microcontroller. This include file defines
// MACROS for special function registers (SFR) and control bits within those
// registers.

// Nicolas Fajardo, Kevin Morris, & Paul Cross
// Team 202
// Date modified: 09/22/2014

// K.N.P. -- Know Nonsense Programmers

#include "p24fj64ga002.h"
#include <stdio.h>
#include "lcd.h"

volatile int state = 0;
volatile unsigned int minutes = 0;
volatile unsigned int seconds = 0;

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

	// TODO: Configure AD1PCFG register for configuring input pins between analog input
	// and digital IO.
	// DONE

	AD1PCFGbits.PCFG4 = 1;

	// TODO: Configure TRIS register bits for Right and Left LED outputs.
	// DONE
	TRISAbits.TRISA2 = 0;	// STOP LED
	TRISAbits.TRISA3 = 0;	// RUN LED

	// TODO: Configure LAT register bits to initialize Right LED to on.
	// DONE
	LATAbits.LATA2 = 1; // LEFT LED OFF
	LATAbits.LATA3 = 0; // RIGHT LED ON

	// TODO: Configure ODC register bits to use open drain configuration for Right
	// and Left LED output.
	// DONE
	ODCAbits.ODA2 = 1;
	ODCAbits.ODA3 = 1;	

	// TODO: Configure TRIS register bits for switch input.
	// DONE
	TRISBbits.TRISB6 = 1;
	TRISBbits.TRISB5 = 1;

	// TODO: Configure CNPU register bits to enable internal pullup resistor for switch input.
	// DONE
	CNPU1bits.CN6PUE = 1;
	
	// This line sets the Timer 2 to use the internal clock, to have a prescaler of 256,
	// and to start in the 'off' position.
	T2CON = 0x0030;

	// TODO: Setup Timer 2 to use internal clock (Fosc/2).
	// _TCS = 0;

	// TODO: Setup Timer 2's prescaler to 1:256.
	// DONE
	// T1CONbits.TCKPS0 = 1;	
	// T1CONbits.TCKPS1 = 1;

 	// TODO: Set Timer 2 to be initially off.
	// DONE	
	// _TON = 0;

	// TODO: Clear Timer 2 value and reset interrupt flag
	// DONE
	TMR2 = 0;
	IFS0bits.T2IF = 0;
	IEC0bits.T2IE = 1;

	// TODO: Set Timer 1's period value register to value for 5 ms.
	// DONE 
	PR2 = 71;

	// set change interrupt for both switches

	CNEN2bits.CN27IE = 1;
	CNEN2bits.CN24IE = 1;
	IFS1bits.CNIF = 0;
	IEC1bits.CNIE = 1;

	while(1) {
		// TODO: For each distinct button press, alternate which
		// LED is illuminated (on).

		switch (state) {
			case 0:
					LCDMoveCursor(0,0);
					LCDPrintString("Stop:");
					LCDMoveCursor(1,0);
					LCDPrintString("00:00.00");
					LATAbits.LATA2 = 1;
					LATAbits.LATA3 = 0;
				break;
			case 1:
					LCDMoveCursor(0,0);
					LCDPrintString("Run:");
					LCDMoveCursor(1,4);
					LCDPrintChar(cnt+'0');
					LATAbits.LATA2 = 0;
					LATAbits.LATA3 = 1;
				break;
			case 2:
					LCDMoveCursor(0,0);
					LCDPrintString("Stop:");
					LATAbits.LATA2 = 1;
					LATAbits.LATA3 = 0;			// stay
				break;
			
		}
		

		// TODO: Use DebounceDelay() function to debounce button press
		// and button release in software.
	}
	return 0;
}

// *******************************************************************************************

void DebounceDelay(void) {	// function declaration for debouncing
	T2CONbits.TON = 1;				// turn on TMR2 to activate 5ms interrupt cycle
}

// verbose call for TMR2 interrupt
void __attribute__((interrupt,auto_psv)) _T2Interrupt(void){

    TMR2 = 0;				// reset TMR1 value
    IFS0bits.T2IF = 0;		// drop interrupt flag to be ready for next interrupt
	T2CONbits.TON = 0;				// turn off TMR2 to ensure no unnecessary interrupt calls
}

void __attribute__((interrupt,auto_psv)) _CNInterrupt(void){
	
	IFS1bits.CNIF == 0;
	DebounceDelay();

	switch (state){
		case 0:								// Initial state
			LCDClear();
			if(PORTBbits.RB5 == 0){
				state = 0;
			}
			else if(PORTBbits.RB6 == 0){
				state = 1;
			}
			break;
		case 1:								// After switch is pressed, before second press
			if(PORTBbits.RB6 == 0){
				state = 2;
			}
			break;
		case 2:								// After switch is pressed second time, before release
			if(PORTBbits.RB6 == 0){
				state = 1;
			}
			break;
	
	}

}