// ******************************************************************************************* //
// Include file for PIC24FJ64GA002 microcontroller. This include file defines
// MACROS for special function registers (SFR) and control bits within those
// registers.

// Nicolas Fajardo & Kevin Morris
// Team 202
// Date modified: 09/22/2014

// K.N.P. -- Know Nonsense Programmers

#include "p24fj64ga002.h"
#include <stdio.h>

volatile int state = 0;

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

int main(void)
{
	// ****************************************************************************** //

	// TODO: Configure AD1PCFG register for configuring input pins between analog input
	// and digital IO.
	// DONE

	AD1PCFGbits.PCFG4 = 1;

	// TODO: Configure TRIS register bits for Right and Left LED outputs.
	// DONE
	TRISAbits.TRISA0 = 0;	// STOP LED
	TRISAbits.TRISA1 = 0;	// RUN LED

	// TODO: Configure LAT register bits to initialize Right LED to on.
	// DONE
	LATAbits.LATA0 = 1; // LEFT LED OFF
	LATAbits.LATA1 = 0; // RIGHT LED ON

	// TODO: Configure ODC register bits to use open drain configuration for Right
	// and Left LED output.
	// DONE
	ODCAbits.ODA0 = 1;
	ODCAbits.ODA1 = 1;	

	// TODO: Configure TRIS register bits for switch input.
	// DONE
	TRISBbits.TRISB2 = 1;

	// TODO: Configure CNPU register bits to enable internal pullup resistor for switch input.
	// DONE
	CNPU1bits.CN6PUE = 1;
	
	// This line sets the Timer 1 to use the internal clock, to have a prescaler of 256,
	// and to start in the 'off' position.
	T1CON = 0x0030;

	// TODO: Setup Timer 1 to use internal clock (Fosc/2).
	// _TCS = 0;

	// TODO: Setup Timer 1's prescaler to 1:256.
	// DONE
	// T1CONbits.TCKPS0 = 1;	
	// T1CONbits.TCKPS1 = 1;

 	// TODO: Set Timer 1 to be initially off.
	// DONE	
	// _TON = 0;

	// TODO: Clear Timer 1 value and reset interrupt flag
	// DONE
	TMR1 = 0;
	IFS0bits.T1IF = 0;
	IEC0bits.T1IE = 1;

	// TODO: Set Timer 1's period value register to value for 5 ms.
	// DONE 
	PR1 = 71;

	while(1) {
		// TODO: For each distinct button press, alternate which
		// LED is illuminated (on).

		switch (state) {
			case 0:
				if (PORTBbits.RB2 == 0) { 	// button press?
					DebounceDelay();		// debounce press
					state = 1;				
				}
				break;
			case 1:
				// if else to check conditions/switch leds
				if (LATAbits.LATA0 == 1) {
					LATAbits.LATA0 = 0;
					LATAbits.LATA1 = 1;
				}
				else {
					LATAbits.LATA0 = 1;
					LATAbits.LATA1 = 0;
				}
				state = 2;
				break;
			case 2:
				if (PORTBbits.RB2 == 1) {	// if button release
					DebounceDelay();		// debounce release
					state = 0;				// return to 0 to wait for press
				}
				else						// else, not released
					state = 2;				// stay
				break;
		}


		// TODO: Use DebounceDelay() function to debounce button press
		// and button release in software.
	}
	return 0;
}

// *******************************************************************************************

void DebounceDelay(void) {	// function declaration for debouncing
	_TON = 1;				// turn on TMR1 to activate 5ms interrupt cycle
}

// verbose call for TMR1 interrupt
void __attribute__((interrupt,auto_psv)) _T1Interrupt(void){

    TMR1 = 0;				// reset TMR1 value
    IFS0bits.T1IF = 0;		// drop interrupt flag to be ready for next interrupt
	_TON = 0;				// turn off TMR1 to ensure no unnecessary interrupt calls
}