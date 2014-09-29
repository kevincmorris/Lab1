// ******************************************************************************************* //

#include "p24fj64ga002.h"

// ******************************************************************************************* //

// LCD Data connected to RB15 -> RB12, RS is connected to RB7, and E is connected to RB6
#define LCD_D   LATB
#define LCD_RS  LATBbits.LATB7
#define LCD_E   LATBbits.LATB6

// ******************************************************************************************* //

// TRIS register mappings for LCD signals
#define LCD_TRIS_D7  TRISBbits.TRISB15
#define LCD_TRIS_D6  TRISBbits.TRISB14
#define LCD_TRIS_D5  TRISBbits.TRISB13
#define LCD_TRIS_D4  TRISBbits.TRISB12
#define LCD_TRIS_RS  TRISBbits.TRISB7
#define LCD_TRIS_E   TRISBbits.TRISB6

// ******************************************************************************************* //

// Defines LCD write types for EnableLCD function that will be assigned to RS LCD input
// used to specific writing control instructions or data characters.
#define LCD_WRITE_DATA    1
#define LCD_WRITE_CONTROL 0

// ******************************************************************************************* //

// TODO: This function will use a 16-bit timer (Timer 2) to wait for approximately 0 to the 16000 us
// specfied by the input usDelay.
//
// Function Inputs:
//    unsigned usDelay     : Specifies the requested delay in microsecond

void DelayUs(unsigned int usDelay) { // DONE
	
	// TODO: Use Timer 2 to delay for precisely (as precise as possible) usDelay
	// microseconds provided by the input variable.
	//
	// Hint: Determine the configuration for the PR1 setting that provides for a
	// one microsecond delay, and multiply this by the input variable.
	// Be sure to user integer values only.
/*****************************************************/
/*
	T2CON = 0x8010;
	- sets TMR1 to be on and use the internal oscillator, with a prescaler of 8
	
	F_CY = 4*F_OSC/2 = 4*7372800/2 = 14745600
	TMR2_FREQ = F_CY/PRESCALER = 1843200
	1843200 * (1 us) = 1.8432 = ~2
*/
	T2CON = 0x8010;
	TMR2 = 0;
	PR2 = (2 * usDelay) - 1;
	IFS0bits.T2IF = 0;
	IEC0bits.T2IE = 1;
	

/*****************************************************/
}

// ******************************************************************************************* //

// EnableLCD function enables writing control instructions or data (ASCII characters) to the
// LCD, controlling the LCD_RS and LCD_E signals.
//
// If the commandType is 0 (LCD_WRITE_CONTROL), the 8-bit control instructions must be
// asserted using the D7-D4 inputs. The 8-bit control instructions must be written
// to the LCD using the D7-D4 inputs as two 4-bit writes with the most significant 4-bits
// (nibble) written first. This requires two calls to the EnableLCD() function.
//
// If the commandType is 1 (LCD_WRITE_DATA), the 8-bit ASCII characters written to the LCD
// will be displayed by the LCD at the current cursor location. The 8-bit characters must be
// written to the LCD using the D7-D4 inputs as two 4-bit writes with the most significant
// 4-bits (nibble) written first. This requires two calls to the EnableLCD() function.

void EnableLCD(unsigned char commandType, unsigned usDelay) {

	LCD_RS = commandType; DelayUs(usDelay);
	LCD_E = 1;  DelayUs(usDelay);
	LCD_E = 0;  DelayUs(usDelay);
}

// ******************************************************************************************* //

// TODO: WriteLCD function should write either control instruciton of data characters to the
// LCD display using the 4-bit interface. This function will handle the bit masking and shifting
// to write the individual 4-bit nibbles to the LCD data inputs. Control instructions
// are written in the commandType is 0 (LCD_WRITE_CONTROL), and ASCII characters are written
// if the commandType is 1 (LCD_WRITE_DATA).
//
// Function Inputs:
//    unsigned char word   : 8-bit control instruction of data character to be written
//    unsigned commandType : Specifies the type of write operation, see above
//    unsigned usDelay     : Specifies the write delay required for this operation

void WriteLCD(unsigned char word, unsigned commandType, unsigned usDelay) {

	// TODO: Using bit masking and shift operations, write most significant bits to correct
	// bits of the LCD_D signal (i.e. #define used to map internal name to LATB)
	// and enable the LCD for the correct command.


	LCD_D = (0xFFF0 & word);
	EnableLCD(commandType, usDelay);

	// TODO: Using bit masking and shift operations, write least significant bits to correct
	// bits of the LCD_D signal (i.e. #define used to map internal name to LATB)
	// and enable the LCD for the correct command.
	

	LCD_D = ((0x0FFFF & word) << 12);
	EnableLCD(commandType, usDelay);
}

// ******************************************************************************************* //

// The initialization routine is utulized to start the LCD operation and configure the
// LCD interface to utilize 4-bit interface. Please reference the data sheet for the LCD
// for further details.

void LCDInitialize(void) {

	// Setup D, RS, and E to be outputs (0).
	LCD_TRIS_D7 = 0;
	LCD_TRIS_D6 = 0;
	LCD_TRIS_D5 = 0;
	LCD_TRIS_D4 = 0;
	LCD_TRIS_RS = 0;
	LCD_TRIS_E = 0;

	LCD_D = (LCD_D & 0x0FFF) | 0x0000;	
	LCD_RS = 0;
	LCD_E = 0;
	DelayUs(15000);		

	LCD_D = (LCD_D & 0x0FFF) | 0x3000; 
	EnableLCD(LCD_WRITE_CONTROL, 4100);


	// Initilization sequence utilizes specific LCD commands before the general configuration
	// commands can be utilized. The first few initialization commands cannot be done using the
	// WriteLCD function. Additionally, the specific sequence and timing is very important.

	// Enable 4-bit interface

	// Function Set (specifies data width, lines, and font.

	// 4-bit mode initialization is complete. We can now configure the various LCD
	// options to control how the LCD will function.

	// TODO: Display On/Off Control
	// Turn Display (D) Off
	WriteLCD(0x08, LCD_WRITE_CONTROL, 37);


	// TODO: Clear Display
	WriteLCD(0x01, LCD_WRITE_CONTROL, 1520);

	// TODO: Entry Mode Set
	// Set Increment Display, No Shift (i.e. cursor move)
	//*** not sure what entry mode is? ***//
//	WriteLCD(0x14, LCD_WRITE_CONTROL, 50);	// setting cursor address to the right of lcd

	WriteLCD(0x07, LCD_WRITE_CONTROL, 37);	// set display to shift left, cursor stationary

	// TODO: Display On/Off Control
	// Turn Display (D) On, Cursor (C) Off, and Blink(B) Off
	WriteLCD(0x0C, LCD_WRITE_CONTROL, 50);


}

// ******************************************************************************************* //

// TODO: LCDClear sends a clear screen command to the LCD tht will both clear the screen
// and return the cursor to the origin (0,0).

void LCDClear(void) {
	
	// TODO: Write the proper control instruction to clear the screen ensuring
	// the proper delay is utilized.

	WriteLCD(0x01, LCD_WRITE_CONTROL, 1520);
	WriteLCD(0x02, LCD_WRITE_CONTROL, 1520);
}

// ******************************************************************************************* //

// TODO: LCDMoveCursor should move to the cursor to the specified (x,y) coordinate. Note that
// as the LCD controller is a generic interface for many LCD displays or varying size,
// some (x,y) cooridnates may not be visible by the LCD display.
//
// Function Inputs:
//    unsigned char x : coordinate for LCD row (0 or 1)
//    unsigned char y : coordinate for LCD column (0 to 7)

void LCDMoveCursor(unsigned char x, unsigned char y) {

	// TODO: Write the propoer control instruction to move the cursor to the specified
	// (x,y) coordinate. This operation should be performance as a single control
	// control instruction, i.e. a single call the WriteLCD() function.

	WriteLCD(0x80 | (x<<6) | y, LCD_WRITE_CONTROL, 37);
	
}

// ******************************************************************************************* //

// TODO: LCDPrintChar should print a single ASCII character to the LCD diplay at the
// current cursor position.
//
// Function Inputs:
//    char c : ASCII character to write to LCD

void LCDPrintChar(char c) {

	// TODO: Write the ASCII character provide as input to the LCD display ensuring
	// the proper delay is utilized.
	WriteLCD(c, LCD_WRITE_DATA, 37);
}

// ******************************************************************************************* //

// LCDPrintString will print a null-terminated string to the LCD at the current cursor
// location.
//
// Notes:
//       1. If the string is longer than the length of a row, the entire string may not
//          appear on the LCD.
//       2. If the string is not null-terminated, random data within the memory pointed to
//          by the pointer will be displayed. This process will only terminate when a null
//          characters if found.

void LCDPrintString(const char* s) {
	int i = 0;
	
	for (i = 0; s[i] != '\0'; ++i)  
		LCDPrintChar(s[i]);
}

// ******************************************************************************************* //
